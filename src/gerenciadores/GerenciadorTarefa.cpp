#include "gerenciadores/GerenciadorTarefa.hpp"
#include "escalonadores/PriopEscalonador.hpp"
#include "escalonadores/SRTFEscalonador.hpp"

#include <algorithm>
#include <numeric>

GerenciadorTarefa* GerenciadorTarefa::instance = nullptr;

// ─── Ciclo de vida estático ────────────────────────────────────────────────────

GerenciadorTarefa* GerenciadorTarefa::getInstance() { return instance; }

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

    // historico[0] = estado inicial (nenhum tick executado)
    historico.push_back(buildSnapshot());
}

GerenciadorTarefa::~GerenciadorTarefa() { delete pEscalonador; }

// ─── Navegação ────────────────────────────────────────────────────────────────

bool GerenciadorTarefa::podeAvancar() const
{
    return tickAtual < (int)historico.size() - 1 || !simulacaoCompleta;
}

bool GerenciadorTarefa::podeRetroceder() const { return tickAtual > 0; }

bool GerenciadorTarefa::isSimulacaoCompleta() const
{
    return simulacaoCompleta && tickAtual == (int)historico.size() - 1;
}

void GerenciadorTarefa::avancar()
{
    if (tickAtual < (int)historico.size() - 1) {
        // Redo: restaura tick já calculado
        tickAtual++;
        aplicarEstado(historico[tickAtual]);
        return;
    }
    if (simulacaoCompleta) return;
    computarProximoTick();
}

void GerenciadorTarefa::retroceder()
{
    if (tickAtual <= 0) return;
    tickAtual--;
    aplicarEstado(historico[tickAtual]);
}

void GerenciadorTarefa::executarCompleto()
{
    int limite = tickLimite();
    while (!simulacaoCompleta && tickAtual < limite)
        avancar();
}

// ─── Edição manual ────────────────────────────────────────────────────────────

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

    // 2. Preempção por quantum expirado (quantumRestante chegou a 0 no tick anterior)
    for (auto& t : listaTarefas) {
        if (t.getEstadoAtual() == EstadoTarefa::Execucao && t.getQuantumRestante() == 0) {
            t.setEstadoAtual(EstadoTarefa::Pronta);
            for (auto& cpu : cpus)
                if (cpu.tarefaAtualID == t.getID())
                    cpu.tarefaAtualID = -1;
        }
    }

    // 3. Chegada de novas tarefas
    for (auto& t : listaTarefas)
        if (t.getEstadoAtual() == EstadoTarefa::Nova && t.getIngresso() <= T)
            t.setEstadoAtual(EstadoTarefa::Pronta);

    // 4. Chama o escalonador com o estado completo atual
    std::map<int, int> novaAlocacao = pEscalonador->escalonar(listaTarefas, cpus, T);

    // 5. Aplica decisões do escalonador (detecta preempções voluntárias)
    for (auto& [cpuId, tarefaId] : novaAlocacao) {
        CPU* cpu = findCPU(cpuId);
        if (!cpu) continue;

        if (tarefaId == cpu->tarefaAtualID) {
            // Mesma tarefa: verifica se quantum precisa ser reiniciado (nova atribuição após preempção)
            // Se tarefaId == -1, CPU continua ociosa — não faz nada
            continue;
        }

        // Tarefa mudou: preempção da tarefa anterior (se havia uma)
        if (cpu->tarefaAtualID != -1) {
            Tarefa* anterior = findTarefa(cpu->tarefaAtualID);
            if (anterior && anterior->getEstadoAtual() == EstadoTarefa::Execucao)
                anterior->setEstadoAtual(EstadoTarefa::Pronta);
        }

        cpu->tarefaAtualID = tarefaId;

        if (tarefaId == -1) {
            // CPU sem tarefa neste tick: desliga se não há nada pronto
            cpu->ligada = hasTarefaProntaOuExecutando();
        } else {
            Tarefa* nova = findTarefa(tarefaId);
            if (nova) {
                nova->setEstadoAtual(EstadoTarefa::Execucao);
                nova->setQuantumRestante(quantum);  // reseta quantum ao assumir CPU
                cpu->ligada = true;
            }
        }
    }

    // 6. Executa as tarefas deste tick (decrementa contadores)
    for (auto& t : listaTarefas) {
        if (t.getEstadoAtual() == EstadoTarefa::Execucao) {
            t.decrementarTempoRestante();
            t.decrementarQuantumRestante();
        }
    }

    // 7. Registra estado de cada tarefa no Gráfico de Gantt
    for (auto& t : listaTarefas)
        t.registrarEstadoNoTempo(T, t.getEstadoAtual());

    // 8. Avança clock e salva snapshot
    tickAtual = T;
    if (todasTerminadas()) simulacaoCompleta = true;
    historico.push_back(buildSnapshot());
}

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

EstadoSistema GerenciadorTarefa::buildSnapshot() const
{
    EstadoSistema snap;
    snap.tempoClock = tickAtual;

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

bool GerenciadorTarefa::hasTarefaProntaOuExecutando() const
{
    for (const auto& t : listaTarefas) {
        auto s = t.getEstadoAtual();
        if (s == EstadoTarefa::Pronta || s == EstadoTarefa::Execucao || s == EstadoTarefa::Nova)
            return true;
    }
    return false;
}

int GerenciadorTarefa::tickLimite() const
{
    int soma = 0;
    for (const auto& t : listaTarefas)
        soma += t.getDuracao();
    int maxIngresso = 0;
    for (const auto& t : listaTarefas)
        maxIngresso = std::max(maxIngresso, t.getIngresso());
    // Margem generosa para evitar loop infinito em caso de bug no escalonador
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
    return new PriopEscalonador();  // padrão: "priop"
}

// ─── Getters ─────────────────────────────────────────────────────────────────

int                        GerenciadorTarefa::getTickAtual()  const { return tickAtual; }
int                        GerenciadorTarefa::getQuantum()    const { return quantum; }
int                        GerenciadorTarefa::getQtdeCpus()   const { return (int)cpus.size(); }
const std::vector<CPU>&    GerenciadorTarefa::getCPUs()       const { return cpus; }
const std::vector<Tarefa>& GerenciadorTarefa::getTarefas()    const { return listaTarefas; }
