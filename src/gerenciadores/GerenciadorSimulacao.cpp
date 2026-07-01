#include "gerenciadores/GerenciadorSimulacao.hpp"
#include "escalonadores/PriopDEscalonador.hpp"
#include "escalonadores/PriopEscalonador.hpp"
#include "escalonadores/SRTFEscalonador.hpp"
#include <algorithm>
#include <numeric>

GerenciadorSimulacao* GerenciadorSimulacao::instance = nullptr;

GerenciadorSimulacao* GerenciadorSimulacao::getInstance() { return instance; }

// cria a instância única a partir de uma configuração carregada do arquivo.
// chamar configurar() duas vezes reinicia a simulação do zero.
void GerenciadorSimulacao::configurar(const ConfigSimulacao& config)
{
    resetar();
    instance = new GerenciadorSimulacao(config);
}

void GerenciadorSimulacao::resetar()
{
    delete instance;
    instance = nullptr;
}

// construtor / destrutor
GerenciadorSimulacao::GerenciadorSimulacao(const ConfigSimulacao& config)
    : pEscalonador(criarEscalonador(config.algoritmo, config.alpha)),
      quantum(config.quantum),
      listaTarefas(config.tarefas),
      tickAtual(0),
      simulacaoCompleta(false)
{
    for (int i = 0; i < config.qtde_cpus; ++i)
        cpus.push_back({i, "", true});

    inicializarMutexes();

    // historico[0] representa o estado antes de qualquer tick ser executado.
    // a cada avanço, um novo snapshot é empilhado em historico[T].
    historico.push_back(buildSnapshot());
}

GerenciadorSimulacao::~GerenciadorSimulacao() { delete pEscalonador; }

// navegacao
// podeAvancar() retorna true se há um tick futuro já calculado (redo) OU
// se a simulação ainda não acabou (próximo tick será calculado sob demanda).
bool GerenciadorSimulacao::podeAvancar() const
{
    return tickAtual < (int)historico.size() - 1 || !simulacaoCompleta;
}

bool GerenciadorSimulacao::podeRetroceder() const { return tickAtual > 0; }

bool GerenciadorSimulacao::isSimulacaoCompleta() const
{
    return simulacaoCompleta && tickAtual == (int)historico.size() - 1;
}

// avança um tick
void GerenciadorSimulacao::avancar()
{
    if (tickAtual < (int)historico.size() - 1) {
        tickAtual++;
        aplicarEstado(historico[tickAtual]);
        return;
    }
    if (simulacaoCompleta) return;
    computarProximoTick();
}

// retrocede um tick restaurando o snapshot salvo em historico[tickAtual-1].
// snapshots futuros permanecem no vetor, permitindo redo
void GerenciadorSimulacao::retroceder()
{
    if (tickAtual <= 0) return;
    tickAtual--;
    aplicarEstado(historico[tickAtual]);
}

// executa todos os ticks restantes de uma vez
void GerenciadorSimulacao::executarCompleto()
{
    int limite = tickLimite();
    while (!simulacaoCompleta && tickAtual < limite)
        avancar();
}

// edicao manual
// snapshots futuros são descartados
void GerenciadorSimulacao::editarEstadoTarefa(const std::string& tarefaId, EstadoTarefa novoEstado)
{
    Tarefa* t = findTarefa(tarefaId);
    if (!t) return;

    // se tarefa em execução vira outra coisa, libera CPU
    if (t->getEstadoAtual() == EstadoTarefa::Execucao
        && novoEstado != EstadoTarefa::Execucao)
    {
        for (auto& cpu : cpus)
            if (cpu.tarefaAtualID == tarefaId)
                cpu.tarefaAtualID = "";
    }

    t->setEstadoAtual(novoEstado);
    t->setMotivoSuspensao(novoEstado == EstadoTarefa::Suspensa
                          ? MotivoSuspensao::Manual
                          : MotivoSuspensao::Nenhum);

    // invalida história futura
    historico.resize(tickAtual + 1);
    simulacaoCompleta = false;
}

// calcula o que acontece no próximo tick
// finalizações e preempções acontecem antes do
// escalonador decidir quem entra, e os decrementos ocorrem só no final
void GerenciadorSimulacao::computarProximoTick()
{
    int T = tickAtual + 1;
    bool escalonadorPreemptivo = pEscalonador->isPreemptivo();
    bool deveReescalonar = false;
    std::vector<std::string> sorteadasTick;
    std::vector<EventoGantt> eventosTick;

    auto temTarefaPronta = [&]() {
        for (const auto& t : listaTarefas)
            if (t.getEstadoAtual() == EstadoTarefa::Pronta)
                return true;
        return false;
    };

    auto temCpuLivre = [&]() {
        for (const auto& cpu : cpus)
            if (cpu.tarefaAtualID.empty())
                return true;
        return false;
    };

    auto aplicarEscalonamento = [&]() {
        ResultadoEscalonamento res = pEscalonador->escalonar(listaTarefas, cpus, T);
        sorteadasTick.insert(sorteadasTick.end(), res.sorteadas.begin(), res.sorteadas.end());

        // aplica as decisões do escalonador: preempções e novas atribuições
        for (auto& [cpuId, tarefaId] : res.alocacao) {
            CPU* cpu = findCPU(cpuId);
            if (!cpu) continue;

            Tarefa* escolhida = tarefaId.empty() ? nullptr : findTarefa(tarefaId);
            if (!tarefaId.empty() && !escolhida) continue;

            if (escolhida && tarefaId == cpu->tarefaAtualID
                && escolhida->getEstadoAtual() == EstadoTarefa::Execucao) {
                // mesma tarefa continua executando: quantum segue contando
                continue;
            }

            // tarefa mudou: preempta a tarefa anterior se havia uma rodando
            if (!cpu->tarefaAtualID.empty()) {
                Tarefa* anterior = findTarefa(cpu->tarefaAtualID);
                if (anterior && anterior->getEstadoAtual() == EstadoTarefa::Execucao) {
                    anterior->setEstadoAtual(EstadoTarefa::Pronta);
                    anterior->setMotivoSuspensao(MotivoSuspensao::Nenhum);
                }
            }

            cpu->tarefaAtualID = tarefaId;

            if (tarefaId.empty()) {
                // CPU sem tarefa: desliga
                cpu->ligada = false;
            } else if (escolhida) {
                escolhida->setEstadoAtual(EstadoTarefa::Execucao);
                escolhida->setMotivoSuspensao(MotivoSuspensao::Nenhum);
                escolhida->setQuantumRestante(quantum);  // reinicia o quantum a cada nova atribuição
                cpu->ligada = true;
            }
        }
    };

    // Ações no instante atual da tarefa acontecem antes de término, quantum ou nova escolha.
    // Isso cobre casos como MUxx no instante final da execução da tarefa.
    if (processarAcoesMutex(eventosTick))
        deveReescalonar = true;

    // tarefas que finalizaram ao final do tick anterior
    for (auto& t : listaTarefas) {
        if (t.getEstadoAtual() == EstadoTarefa::Execucao && t.getTempoRestante() == 0) {
            t.setEstadoAtual(EstadoTarefa::Terminada);
            t.setMotivoSuspensao(MotivoSuspensao::Nenhum);
            deveReescalonar = true;
            for (auto& cpu : cpus)
                if (cpu.tarefaAtualID == t.getID())
                    cpu.tarefaAtualID = "";
        }
    }

    // Em escalonadores preemptivos, quantum expirado abre um ponto de reescalonamento.
    // Em cooperativos, a tarefa continua executando até terminar.
    if (escalonadorPreemptivo) {
        for (auto& t : listaTarefas) {
            if (t.getEstadoAtual() == EstadoTarefa::Execucao && t.getQuantumRestante() == 0) {
                t.setEstadoAtual(EstadoTarefa::Pronta);
                t.setMotivoSuspensao(MotivoSuspensao::Nenhum);
                deveReescalonar = true;
                // não limpa cpu.tarefaAtualID para preservar a afinidade CPU↔tarefa
                // o escalonador usa esse vínculo para evitar troca de CPU desnecessária
            }
        }
    }

    // ingresso=0 fica pronta no tick 1, ingresso=1 no tick 2, etc.
    for (auto& t : listaTarefas)
        if (t.getEstadoAtual() == EstadoTarefa::Nova && t.getIngresso() < T) {
            t.setEstadoAtual(EstadoTarefa::Pronta);
            t.setMotivoSuspensao(MotivoSuspensao::Nenhum);
        }

    if (temTarefaPronta() && temCpuLivre())
        deveReescalonar = true;

    int guarda = 0;
    while (guarda++ < (int)listaTarefas.size() + (int)cpus.size() + 10) {
        if (deveReescalonar) {
            // chama o escalonador somente quando há um evento que permite nova decisão
            aplicarEscalonamento();
            deveReescalonar = false;
        }

        bool acoesPedemReescalonamento = processarAcoesMutex(eventosTick);
        if (!acoesPedemReescalonamento)
            break;

        if (temTarefaPronta() && (escalonadorPreemptivo || temCpuLivre()))
            deveReescalonar = true;
        else
            break;
    }

    // executa as tarefas deste tick: decrementa tempo restante e quantum restante
    for (auto& t : listaTarefas) {
        if (t.getEstadoAtual() == EstadoTarefa::Execucao) {
            t.decrementarTempoRestante();
            t.decrementarQuantumRestante();
        }
    }

    // registra o estado de cada tarefa no tick T
    for (auto& t : listaTarefas)
        t.registrarEstadoNoTempo(T, t.getEstadoAtual());

    // avança o relógio, verifica término e salva snapshot para undo/redo
    tickAtual = T;
    if (todasTerminadas()) simulacaoCompleta = true;
    historico.push_back(buildSnapshot(sorteadasTick, eventosTick));
}

// restaura o estado do sistema a partir de um snapshot
void GerenciadorSimulacao::aplicarEstado(const EstadoSistema& estado)
{
    for (const auto& snap : estado.tarefas) {
        Tarefa* t = findTarefa(snap.id);
        if (!t) continue;
        t->setEstadoAtual(snap.estado);
        t->setTempoRestante(snap.tempoRestante);
        t->setQuantumRestante(snap.quantumRestante);
        t->setPrioridadeDinamica(snap.prioridadeDinamica);
        t->setProximaAcaoIndex(snap.proximaAcaoIndex);
        t->setMotivoSuspensao(snap.motivoSuspensao);
    }

    mutexes.clear();
    for (const auto& snap : estado.mutexes) {
        Mutex mutex;
        mutex.id = snap.id;
        mutex.donoTarefaID = snap.donoTarefaID;
        for (const auto& tarefaId : snap.filaEspera)
            mutex.filaEspera.push_back(tarefaId);
        mutexes.push_back(mutex);
    }

    for (auto& cpu : cpus) {
        auto itA = estado.alocacaoCPU.find(cpu.id);
        auto itL = estado.cpuLigada.find(cpu.id);
        cpu.tarefaAtualID = (itA != estado.alocacaoCPU.end()) ? itA->second : "";
        cpu.ligada        = (itL != estado.cpuLigada.end())   ? itL->second : true;
    }

    tickAtual = estado.tempoClock;
    simulacaoCompleta = (tickAtual == (int)historico.size() - 1) && todasTerminadas();
}

// constrói snapshot completo do estado atual do sistema.
EstadoSistema GerenciadorSimulacao::buildSnapshot(const std::vector<std::string>& sorteadas,
                                                  const std::vector<EventoGantt>& eventos) const
{
    EstadoSistema snap;
    snap.tempoClock = tickAtual;
    snap.sorteadas  = sorteadas;
    snap.eventos    = eventos;

    for (const auto& t : listaTarefas)
        snap.tarefas.push_back({t.getID(), t.getEstadoAtual(),
                                 t.getMotivoSuspensao(),
                                 t.getTempoRestante(), t.getQuantumRestante(),
                                 t.getPrioridadeDinamica(),
                                 t.getProximaAcaoIndex()});

    for (const auto& cpu : cpus) {
        snap.alocacaoCPU[cpu.id] = cpu.tarefaAtualID;
        snap.cpuLigada[cpu.id]   = cpu.ligada;
    }

    for (const auto& mutex : mutexes) {
        SnapshotMutex snapMutex;
        snapMutex.id = mutex.id;
        snapMutex.donoTarefaID = mutex.donoTarefaID;
        for (const auto& tarefaId : mutex.filaEspera)
            snapMutex.filaEspera.push_back(tarefaId);
        snap.mutexes.push_back(snapMutex);
    }

    return snap;
}

void GerenciadorSimulacao::inicializarMutexes()
{
    for (const auto& tarefa : listaTarefas)
        for (const auto& acao : tarefa.getAcoes())
            getOrCreateMutex(acao.mutexId);
}

bool GerenciadorSimulacao::processarAcoesMutex(std::vector<EventoGantt>& eventos)
{
    bool precisaReescalonar = false;

    for (auto& cpu : cpus) {
        if (cpu.tarefaAtualID.empty())
            continue;

        Tarefa* tarefa = findTarefa(cpu.tarefaAtualID);
        if (!tarefa || tarefa->getEstadoAtual() != EstadoTarefa::Execucao)
            continue;

        bool bloqueou = false;
        while (!bloqueou) {
            std::size_t index = tarefa->getProximaAcaoIndex();
            const auto& acoes = tarefa->getAcoes();
            if (index >= acoes.size())
                break;

            const AcaoTarefa& acao = acoes[index];
            if (acao.tempoRelativo > tarefa->getTempoExecutado())
                break;

            Mutex& mutex = getOrCreateMutex(acao.mutexId);

            if (acao.tipo == TipoAcaoTarefa::SolicitarMutex) {
                eventos.push_back({tarefa->getID(), TipoEventoGantt::SolicitarMutex, acao.mutexId});
                tarefa->avancarAcao();

                if (mutex.donoTarefaID.empty() || mutex.donoTarefaID == tarefa->getID()) {
                    mutex.donoTarefaID = tarefa->getID();
                    continue;
                }

                auto jaEsperando = std::find(mutex.filaEspera.begin(), mutex.filaEspera.end(),
                                             tarefa->getID());
                if (jaEsperando == mutex.filaEspera.end())
                    mutex.filaEspera.push_back(tarefa->getID());

                tarefa->setEstadoAtual(EstadoTarefa::Suspensa);
                tarefa->setMotivoSuspensao(MotivoSuspensao::Mutex);
                cpu.tarefaAtualID = "";
                cpu.ligada = false;
                precisaReescalonar = true;
                bloqueou = true;
            } else {
                eventos.push_back({tarefa->getID(), TipoEventoGantt::LiberarMutex, acao.mutexId});
                tarefa->avancarAcao();

                if (mutex.donoTarefaID != tarefa->getID())
                    continue;

                if (mutex.filaEspera.empty()) {
                    mutex.donoTarefaID.clear();
                    continue;
                }

                std::string proximaTarefaId = mutex.filaEspera.front();
                mutex.filaEspera.pop_front();
                mutex.donoTarefaID = proximaTarefaId;

                Tarefa* proxima = findTarefa(proximaTarefaId);
                if (proxima && proxima->getEstadoAtual() == EstadoTarefa::Suspensa &&
                    proxima->getMotivoSuspensao() == MotivoSuspensao::Mutex) {
                    proxima->setEstadoAtual(EstadoTarefa::Pronta);
                    proxima->setMotivoSuspensao(MotivoSuspensao::Nenhum);
                    precisaReescalonar = true;
                }
            }
        }
    }

    return precisaReescalonar;
}

bool GerenciadorSimulacao::todasTerminadas() const
{
    for (const auto& t : listaTarefas)
        if (t.getEstadoAtual() != EstadoTarefa::Terminada)
            return false;
    return !listaTarefas.empty();
}

// true se ainda há trabalho a fazer, usado para decidir se as CPUs devem ficar ligadas.
bool GerenciadorSimulacao::hasTarefaProntaOuExecutando() const
{
    for (const auto& t : listaTarefas) {
        auto s = t.getEstadoAtual();
        if (s == EstadoTarefa::Pronta || s == EstadoTarefa::Execucao || s == EstadoTarefa::Nova)
            return true;
    }
    return false;
}

// limite superior de segurança para executarCompleto(): soma de todas as durações
// mais o maior ingresso, com margem extra para evitar loop infinito
int GerenciadorSimulacao::tickLimite() const
{
    int soma = 0;
    for (const auto& t : listaTarefas)
        soma += t.getDuracao();
    int maxIngresso = 0;
    for (const auto& t : listaTarefas)
        maxIngresso = std::max(maxIngresso, t.getIngresso());
    return maxIngresso + soma + 10;
}

Tarefa* GerenciadorSimulacao::findTarefa(const std::string& id)
{
    for (auto& t : listaTarefas)
        if (t.getID() == id) return &t;
    return nullptr;
}

CPU* GerenciadorSimulacao::findCPU(int id)
{
    for (auto& cpu : cpus)
        if (cpu.id == id) return &cpu;
    return nullptr;
}

Mutex* GerenciadorSimulacao::findMutex(int id)
{
    for (auto& mutex : mutexes)
        if (mutex.id == id) return &mutex;
    return nullptr;
}

Mutex& GerenciadorSimulacao::getOrCreateMutex(int id)
{
    if (Mutex* mutex = findMutex(id))
        return *mutex;

    mutexes.push_back({id, "", {}});
    return mutexes.back();
}

Escalonador* GerenciadorSimulacao::criarEscalonador(const std::string& tipo, int alpha)
{
    if (tipo == "srtf")   return new SRTFEscalonador();
    if (tipo == "priopd" || tipo == "priopenv") return new PriopDEscalonador(alpha);
    return new PriopEscalonador();
}

int                               GerenciadorSimulacao::getTickAtual()  const { return tickAtual; }
int                               GerenciadorSimulacao::getQuantum()    const { return quantum; }
int                               GerenciadorSimulacao::getQtdeCpus()   const { return (int)cpus.size(); }
const std::vector<CPU>&           GerenciadorSimulacao::getCPUs()       const { return cpus; }
const std::vector<Tarefa>&        GerenciadorSimulacao::getTarefas()    const { return listaTarefas; }
const std::vector<EstadoSistema>& GerenciadorSimulacao::getHistorico()  const { return historico; }
