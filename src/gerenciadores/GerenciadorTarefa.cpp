#include "gerenciadores/GerenciadorTarefa.hpp"
#include "escalonadores/PriopEscalonador.hpp"
#include "escalonadores/SRTFEscalonador.hpp"
#include <algorithm>
#include <numeric>

GerenciadorTarefa* GerenciadorTarefa::instance = nullptr;

GerenciadorTarefa* GerenciadorTarefa::getInstance() { return instance; }

// cria a instância única a partir de uma configuração carregada do arquivo.
// chamar configurar() duas vezes reinicia a simulação do zero.
void GerenciadorTarefa::configurar(const ConfigSimulacao& config)
{
    resetar();
    instance = new GerenciadorTarefa(config);
}

void GerenciadorTarefa::resetar()
{
    delete instance;
    instance = nullptr;
}

// construtor / destrutor
GerenciadorTarefa::GerenciadorTarefa(const ConfigSimulacao& config)
    : pEscalonador(criarEscalonador(config.algoritmo)),
      quantum(config.quantum),
      listaTarefas(config.tarefas),
      tickAtual(0),
      simulacaoCompleta(false)
{
    for (int i = 0; i < config.qtde_cpus; ++i)
        cpus.push_back({i, -1, true});

    // historico[0] representa o estado antes de qualquer tick ser executado.
    // a cada avanço, um novo snapshot é empilhado em historico[T].
    historico.push_back(buildSnapshot());
}

GerenciadorTarefa::~GerenciadorTarefa() { delete pEscalonador; }

// navegacao
// podeAvancar() retorna true se há um tick futuro já calculado (redo) OU
// se a simulação ainda não acabou (próximo tick será calculado sob demanda).
bool GerenciadorTarefa::podeAvancar() const
{
    return tickAtual < (int)historico.size() - 1 || !simulacaoCompleta;
}

bool GerenciadorTarefa::podeRetroceder() const { return tickAtual > 0; }

bool GerenciadorTarefa::isSimulacaoCompleta() const
{
    return simulacaoCompleta && tickAtual == (int)historico.size() - 1;
}

// avança um tick
void GerenciadorTarefa::avancar()
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
void GerenciadorTarefa::retroceder()
{
    if (tickAtual <= 0) return;
    tickAtual--;
    aplicarEstado(historico[tickAtual]);
}

// executa todos os ticks restantes de uma vez
void GerenciadorTarefa::executarCompleto()
{
    int limite = tickLimite();
    while (!simulacaoCompleta && tickAtual < limite)
        avancar();
}

// edicao manual
// snapshots futuros são descartados
void GerenciadorTarefa::editarEstadoTarefa(int tarefaId, EstadoTarefa novoEstado)
{
    Tarefa* t = findTarefa(tarefaId);
    if (!t) return;

    // se tarefa em execução vira outra coisa, libera CPU
    if (t->getEstadoAtual() == EstadoTarefa::Execucao
        && novoEstado != EstadoTarefa::Execucao)
    {
        for (auto& cpu : cpus)
            if (cpu.tarefaAtualID == tarefaId)
                cpu.tarefaAtualID = -1;
    }

    t->setEstadoAtual(novoEstado);

    // invalida história futura
    historico.resize(tickAtual + 1);
    simulacaoCompleta = false;
}

// calcula o que acontece no próximo tick
// finalizações e preempções acontecem antes do
// escalonador decidir quem entra, e os decrementos ocorrem só no final
void GerenciadorTarefa::computarProximoTick()
{
    int T = tickAtual + 1;

    // tarefas que finalizaram ao final do tick anterior
    for (auto& t : listaTarefas) {
        if (t.getEstadoAtual() == EstadoTarefa::Execucao && t.getTempoRestante() == 0) {
            t.setEstadoAtual(EstadoTarefa::Terminada);
            for (auto& cpu : cpus)
                if (cpu.tarefaAtualID == t.getID())
                    cpu.tarefaAtualID = -1;
        }
    }

    // preempção por quantum expirado
    // a tarefa volta para Pronta e a CPU é liberada
    for (auto& t : listaTarefas) {
        if (t.getEstadoAtual() == EstadoTarefa::Execucao && t.getQuantumRestante() == 0) {
            t.setEstadoAtual(EstadoTarefa::Pronta);
            for (auto& cpu : cpus)
                if (cpu.tarefaAtualID == t.getID())
                    cpu.tarefaAtualID = -1;
        }
    }

    // ingresso=0 fica pronta no tick 1, ingresso=1 no tick 2, etc.
    for (auto& t : listaTarefas)
        if (t.getEstadoAtual() == EstadoTarefa::Nova && t.getIngresso() < T)
            t.setEstadoAtual(EstadoTarefa::Pronta);

    // chama o escalonador, que devolve mapa cpu_id -> tarefa_id para este tick, com o estado atual
    ResultadoEscalonamento res = pEscalonador->escalonar(listaTarefas, cpus, T);

    // aplica as decisões do escalonador: preempções e novas atribuições
    for (auto& [cpuId, tarefaId] : res.alocacao) {
        CPU* cpu = findCPU(cpuId);
        if (!cpu) continue;

        if (tarefaId != -1 && tarefaId == cpu->tarefaAtualID) {
            // mesma tarefa continua: quantum segue contando
            continue;
        }

        // tarefa mudou: preempta a tarefa anterior se havia uma rodando
        if (cpu->tarefaAtualID != -1) {
            Tarefa* anterior = findTarefa(cpu->tarefaAtualID);
            if (anterior && anterior->getEstadoAtual() == EstadoTarefa::Execucao)
                anterior->setEstadoAtual(EstadoTarefa::Pronta);
        }

        cpu->tarefaAtualID = tarefaId;

        if (tarefaId == -1) {
            // CPU sem tarefa: desliga
            cpu->ligada = false;
        } else {
            Tarefa* nova = findTarefa(tarefaId);
            if (nova) {
                nova->setEstadoAtual(EstadoTarefa::Execucao);
                nova->setQuantumRestante(quantum);  // reinicia o quantum a cada nova atribuição
                cpu->ligada = true;
            }
        }
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
    historico.push_back(buildSnapshot(res.sorteadas));
}

// restaura o estado do sistema a partir de um snapshot
void GerenciadorTarefa::aplicarEstado(const EstadoSistema& estado)
{
    for (const auto& snap : estado.tarefas) {
        Tarefa* t = findTarefa(snap.id);
        if (!t) continue;
        t->setEstadoAtual(snap.estado);
        t->setTempoRestante(snap.tempoRestante);
        t->setQuantumRestante(snap.quantumRestante);
    }

    for (auto& cpu : cpus) {
        auto itA = estado.alocacaoCPU.find(cpu.id);
        auto itL = estado.cpuLigada.find(cpu.id);
        cpu.tarefaAtualID = (itA != estado.alocacaoCPU.end()) ? itA->second : -1;
        cpu.ligada        = (itL != estado.cpuLigada.end())   ? itL->second : true;
    }

    tickAtual = estado.tempoClock;
    simulacaoCompleta = (tickAtual == (int)historico.size() - 1) && todasTerminadas();
}

// constrói snapshot completo do estado atual do sistema.
EstadoSistema GerenciadorTarefa::buildSnapshot(const std::vector<int>& sorteadas) const
{
    EstadoSistema snap;
    snap.tempoClock = tickAtual;
    snap.sorteadas  = sorteadas;

    for (const auto& t : listaTarefas)
        snap.tarefas.push_back({t.getID(), t.getEstadoAtual(),
                                 t.getTempoRestante(), t.getQuantumRestante()});

    for (const auto& cpu : cpus) {
        snap.alocacaoCPU[cpu.id] = cpu.tarefaAtualID;
        snap.cpuLigada[cpu.id]   = cpu.ligada;
    }

    return snap;
}

bool GerenciadorTarefa::todasTerminadas() const
{
    for (const auto& t : listaTarefas)
        if (t.getEstadoAtual() != EstadoTarefa::Terminada)
            return false;
    return !listaTarefas.empty();
}

// true se ainda há trabalho a fazer, usado para decidir se as CPUs devem ficar ligadas.
bool GerenciadorTarefa::hasTarefaProntaOuExecutando() const
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
int GerenciadorTarefa::tickLimite() const
{
    int soma = 0;
    for (const auto& t : listaTarefas)
        soma += t.getDuracao();
    int maxIngresso = 0;
    for (const auto& t : listaTarefas)
        maxIngresso = std::max(maxIngresso, t.getIngresso());
    return maxIngresso + soma + 10;
}

Tarefa* GerenciadorTarefa::findTarefa(int id)
{
    for (auto& t : listaTarefas)
        if (t.getID() == id) return &t;
    return nullptr;
}

CPU* GerenciadorTarefa::findCPU(int id)
{
    for (auto& cpu : cpus)
        if (cpu.id == id) return &cpu;
    return nullptr;
}

Escalonador* GerenciadorTarefa::criarEscalonador(const std::string& tipo)
{
    if (tipo == "srtf")   return new SRTFEscalonador();
    return new PriopEscalonador();
}

int                               GerenciadorTarefa::getTickAtual()  const { return tickAtual; }
int                               GerenciadorTarefa::getQuantum()    const { return quantum; }
int                               GerenciadorTarefa::getQtdeCpus()   const { return (int)cpus.size(); }
const std::vector<CPU>&           GerenciadorTarefa::getCPUs()       const { return cpus; }
const std::vector<Tarefa>&        GerenciadorTarefa::getTarefas()    const { return listaTarefas; }
const std::vector<EstadoSistema>& GerenciadorTarefa::getHistorico()  const { return historico; }
