#include "gerenciadores/GerenciadorTarefa.hpp"
#include "escalonadores/PriopEscalonador.hpp"
#include "escalonadores/SRTFEscalonador.hpp"

#include <algorithm>
#include <numeric>

GerenciadorTarefa* GerenciadorTarefa::instance = nullptr;

// ─── Ciclo de vida estático ────────────────────────────────────────────────────

GerenciadorTarefa* GerenciadorTarefa::getInstance() { return instance; }

// Cria (ou recria) a instância única a partir de uma configuração carregada do arquivo.
// Chamar configurar() duas vezes reinicia a simulação do zero.
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

// ─── Construtor / destrutor ────────────────────────────────────────────────────

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
    // A cada avanço, um novo snapshot é empilhado em historico[T].
    historico.push_back(buildSnapshot());
}

GerenciadorTarefa::~GerenciadorTarefa() { delete pEscalonador; }

// ─── Navegação ────────────────────────────────────────────────────────────────

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

// Avança um tick: se já existe o próximo snapshot no histórico (redo), apenas
// restaura-o; caso contrário, computa o próximo tick do zero.
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

// Retrocede um tick restaurando o snapshot salvo em historico[tickAtual-1].
// Os snapshots futuros permanecem no vetor, permitindo refazer (redo).
void GerenciadorTarefa::retroceder()
{
    if (tickAtual <= 0) return;
    tickAtual--;
    aplicarEstado(historico[tickAtual]);
}

// Executa todos os ticks restantes de uma vez, respeitando o limite de segurança
// calculado por tickLimite() para evitar loop infinito em caso de bug no escalonador.
void GerenciadorTarefa::executarCompleto()
{
    int limite = tickLimite();
    while (!simulacaoCompleta && tickAtual < limite)
        avancar();
}

// ─── Edição manual ────────────────────────────────────────────────────────────

// Permite alterar o estado de uma tarefa em qualquer ponto da simulação.
// Ao editar, os snapshots futuros são descartados (historico.resize) porque
// o novo estado pode gerar uma sequência completamente diferente de ticks.
void GerenciadorTarefa::editarEstadoTarefa(int tarefaId, EstadoTarefa novoEstado)
{
    Tarefa* t = findTarefa(tarefaId);
    if (!t) return;

    // Se a tarefa estava em execução e vira outra coisa, libera a CPU
    if (t->getEstadoAtual() == EstadoTarefa::Execucao
        && novoEstado != EstadoTarefa::Execucao)
    {
        for (auto& cpu : cpus)
            if (cpu.tarefaAtualID == tarefaId)
                cpu.tarefaAtualID = -1;
    }

    t->setEstadoAtual(novoEstado);

    // Invalida história futura — o usuário mudou o estado, novos ticks serão recalculados
    historico.resize(tickAtual + 1);
    simulacaoCompleta = false;
}

// ─── Motor de simulação ───────────────────────────────────────────────────────

// Núcleo da simulação: calcula o que acontece no próximo tick (T = tickAtual + 1).
// A ordem dos passos é crítica: finalizações e preempções acontecem antes do
// escalonador decidir quem entra, e os decrementos ocorrem só no final.
void GerenciadorTarefa::computarProximoTick()
{
    int T = tickAtual + 1;

    // 1. Tarefas que finalizaram ao final do tick anterior (tempoRestante chegou a 0)
    for (auto& t : listaTarefas) {
        if (t.getEstadoAtual() == EstadoTarefa::Execucao && t.getTempoRestante() == 0) {
            t.setEstadoAtual(EstadoTarefa::Terminada);
            for (auto& cpu : cpus)
                if (cpu.tarefaAtualID == t.getID())
                    cpu.tarefaAtualID = -1;
        }
    }

    // 2. Preempção por quantum expirado (quantumRestante chegou a 0 no tick anterior).
    //    A tarefa volta para Pronta e a CPU é liberada; o escalonador decidirá
    //    se ela volta imediatamente ou cede lugar para outra.
    for (auto& t : listaTarefas) {
        if (t.getEstadoAtual() == EstadoTarefa::Execucao && t.getQuantumRestante() == 0) {
            t.setEstadoAtual(EstadoTarefa::Pronta);
            for (auto& cpu : cpus)
                if (cpu.tarefaAtualID == t.getID())
                    cpu.tarefaAtualID = -1;
        }
    }

    // 3. Chegada de novas tarefas: se o ingresso <= T, a tarefa entra na fila de prontas
    for (auto& t : listaTarefas)
        if (t.getEstadoAtual() == EstadoTarefa::Nova && t.getIngresso() <= T)
            t.setEstadoAtual(EstadoTarefa::Pronta);

    // 4. Chama o escalonador com o estado completo atual.
    //    O escalonador devolve um mapa cpu_id → tarefa_id para este tick.
    ResultadoEscalonamento res = pEscalonador->escalonar(listaTarefas, cpus, T);

    // 5. Aplica as decisões do escalonador: preempções e novas atribuições.
    //    Se a mesma tarefa permanece na mesma CPU, nada muda (evita context switch).
    for (auto& [cpuId, tarefaId] : res.alocacao) {
        CPU* cpu = findCPU(cpuId);
        if (!cpu) continue;

        if (tarefaId == cpu->tarefaAtualID) {
            // Mesma tarefa: verifica se quantum precisa ser reiniciado (nova atribuição após preempção)
            // Se tarefaId == -1, CPU continua ociosa — não faz nada
            continue;
        }

        // Tarefa mudou: preempta a tarefa anterior (se havia uma rodando)
        if (cpu->tarefaAtualID != -1) {
            Tarefa* anterior = findTarefa(cpu->tarefaAtualID);
            if (anterior && anterior->getEstadoAtual() == EstadoTarefa::Execucao)
                anterior->setEstadoAtual(EstadoTarefa::Pronta);
        }

        cpu->tarefaAtualID = tarefaId;

        if (tarefaId == -1) {
            // CPU sem tarefa: desliga se não há nenhuma tarefa pronta ou futura no sistema
            cpu->ligada = hasTarefaProntaOuExecutando();
        } else {
            Tarefa* nova = findTarefa(tarefaId);
            if (nova) {
                nova->setEstadoAtual(EstadoTarefa::Execucao);
                nova->setQuantumRestante(quantum);  // reinicia o quantum a cada nova atribuição
                cpu->ligada = true;
            }
        }
    }

    // 6. Executa as tarefas deste tick: decrementa tempo restante e quantum restante.
    //    Feito após a alocação para que os contadores reflitam o consumo do tick atual.
    for (auto& t : listaTarefas) {
        if (t.getEstadoAtual() == EstadoTarefa::Execucao) {
            t.decrementarTempoRestante();
            t.decrementarQuantumRestante();
        }
    }

    // 7. Registra o estado de cada tarefa no tick T (alimenta o Gráfico de Gantt)
    for (auto& t : listaTarefas)
        t.registrarEstadoNoTempo(T, t.getEstadoAtual());

    // 8. Avança o relógio, verifica término e salva snapshot para undo/redo
    tickAtual = T;
    if (todasTerminadas()) simulacaoCompleta = true;
    historico.push_back(buildSnapshot(res.sorteadas));
}

// Restaura o estado completo do sistema (tarefas + CPUs + clock) a partir de um snapshot.
// Usado tanto pelo retroceder() quanto pelo avancar() em modo redo.
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

// Constrói um snapshot completo do estado atual do sistema.
// O campo 'sorteadas' carrega os IDs das tarefas cujo empate foi resolvido por
// sorteio neste tick — usados para exibir o ícone no Gráfico de Gantt.
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

// ─── Utilitários ──────────────────────────────────────────────────────────────

bool GerenciadorTarefa::todasTerminadas() const
{
    for (const auto& t : listaTarefas)
        if (t.getEstadoAtual() != EstadoTarefa::Terminada)
            return false;
    return !listaTarefas.empty();
}

// Retorna true se ainda há trabalho a fazer: tarefas Prontas, em Execução,
// ou que ainda vão chegar (Nova). Usado para decidir se as CPUs devem ficar ligadas.
bool GerenciadorTarefa::hasTarefaProntaOuExecutando() const
{
    for (const auto& t : listaTarefas) {
        auto s = t.getEstadoAtual();
        if (s == EstadoTarefa::Pronta || s == EstadoTarefa::Execucao || s == EstadoTarefa::Nova)
            return true;
    }
    return false;
}

// Limite superior de segurança para executarCompleto(): soma de todas as durações
// mais o maior ingresso, com margem extra para evitar loop infinito caso o
// escalonador tenha um bug e nunca termine as tarefas.
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

// Instancia o escalonador correto com base na string do arquivo de configuração.
// Qualquer valor desconhecido cai no PRIOp por padrão.
Escalonador* GerenciadorTarefa::criarEscalonador(const std::string& tipo)
{
    if (tipo == "srtf")   return new SRTFEscalonador();
    return new PriopEscalonador();  // padrão: "priop"
}

// ─── Getters ─────────────────────────────────────────────────────────────────

int                               GerenciadorTarefa::getTickAtual()  const { return tickAtual; }
int                               GerenciadorTarefa::getQuantum()    const { return quantum; }
int                               GerenciadorTarefa::getQtdeCpus()   const { return (int)cpus.size(); }
const std::vector<CPU>&           GerenciadorTarefa::getCPUs()       const { return cpus; }
const std::vector<Tarefa>&        GerenciadorTarefa::getTarefas()    const { return listaTarefas; }
const std::vector<EstadoSistema>& GerenciadorTarefa::getHistorico()  const { return historico; }
