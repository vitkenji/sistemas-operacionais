// GerenciadorTarefa.cpp
// Implementação do Singleton que orquestra a simulação.
// Veja GerenciadorTarefa.hpp para a descrição completa das decisões de design.

#include "gerenciadores/GerenciadorTarefa.hpp"
#include "escalonadores/PriopEscalonador.hpp"
#include "escalonadores/SRTFEscalonador.hpp"

#include <algorithm>
#include <numeric>

GerenciadorTarefa* GerenciadorTarefa::instance = nullptr;

// ── Ciclo de vida estático ─────────────────────────────────────────────────────

GerenciadorTarefa* GerenciadorTarefa::getInstance() { return instance; }

// Destrói qualquer instância existente e cria uma nova com a configuração dada.
// Chamado pela TelaInicial ao clicar em "Iniciar Simulação".
void GerenciadorTarefa::configurar(const ConfigSimulacao& config)
{
    resetar();
    instance = new GerenciadorTarefa(config);
}

// Libera a instância atual. Chamado ao voltar para a tela de configuração.
void GerenciadorTarefa::resetar()
{
    delete instance;
    instance = nullptr;
}

// ── Construtor / destrutor ─────────────────────────────────────────────────────

// Inicializa o sistema: cria as CPUs, instancia o escalonador e
// grava o snapshot inicial (tick 0) no histórico.
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

// ── Navegação (req. 1.5.2) ─────────────────────────────────────────────────────

// Há ticks para avançar se ainda não chegamos ao fim do historico calculado
// OU se a simulação não está marcada como completa.
bool GerenciadorTarefa::podeAvancar() const
{
    return tickAtual < (int)historico.size() - 1 || !simulacaoCompleta;
}

bool GerenciadorTarefa::podeRetroceder() const { return tickAtual > 0; }

bool GerenciadorTarefa::isSimulacaoCompleta() const
{
    return simulacaoCompleta && tickAtual == (int)historico.size() - 1;
}

// Avança um tick. Se o tick já foi calculado (redo), apenas restaura o snapshot.
// Caso contrário, computa o próximo tick do zero.
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

// Retrocede um tick restaurando o snapshot anterior (undo).
void GerenciadorTarefa::retroceder()
{
    if (tickAtual <= 0) return;
    tickAtual--;
    aplicarEstado(historico[tickAtual]);
}

// Execução completa (req. 1.5.3): avança até todas as tarefas terminarem,
// sem exibir passos intermediários.
void GerenciadorTarefa::executarCompleto()
{
    int limite = tickLimite();
    while (!simulacaoCompleta && tickAtual < limite)
        avancar();
}

// ── Edição manual (req. 3.4) ──────────────────────────────────────────────────

// Modifica o estado de uma tarefa no tick atual.
// Após a edição, invalida o histórico futuro — os ticks seguintes serão
// recalculados a partir do novo estado na próxima chamada a avancar().
void GerenciadorTarefa::editarEstadoTarefa(int tarefaId, EstadoTarefa novoEstado)
{
    Tarefa* t = findTarefa(tarefaId);
    if (!t) return;

    EstadoTarefa estadoAnterior = t->getEstadoAtual();
    if (estadoAnterior == novoEstado) return;

    // Se a tarefa saiu de Execucao, libera a CPU que ela ocupava
    if (estadoAnterior == EstadoTarefa::Execucao) {
        for (auto& cpu : cpus) {
            if (cpu.tarefaAtualID == tarefaId) {
                cpu.tarefaAtualID = -1;
                // Reavalia se a CPU deve ficar ligada ou desligar
                cpu.ligada = hasTarefaProntaOuExecutando();
                break;
            }
        }
    }

    // Se o usuário quer colocar a tarefa em Execucao, precisa de uma CPU livre.
    // Sem CPU disponível, a mudança é ignorada para evitar estado inconsistente.
    if (novoEstado == EstadoTarefa::Execucao) {
        CPU* cpuLivre = nullptr;
        for (auto& cpu : cpus)
            if (cpu.tarefaAtualID == -1) { cpuLivre = &cpu; break; }

        if (!cpuLivre) return;  // nenhuma CPU disponível — não aplica a edição

        cpuLivre->tarefaAtualID = tarefaId;
        cpuLivre->ligada        = true;
        t->setQuantumRestante(quantum);
    }

    t->setEstadoAtual(novoEstado);

    // Invalida história futura — o usuário mudou o estado, novos ticks serão recalculados
    historico.resize(tickAtual + 1);
    simulacaoCompleta = false;
}

// ── Motor de simulação ────────────────────────────────────────────────────────

// Calcula o próximo tick. Sequência:
//   1. Finaliza tarefas cujo tempoRestante chegou a zero no tick anterior.
//   2. Preempção por quantum expirado (quantumRestante == 0).
//   3. Ativa tarefas que chegam neste tick (Nova → Pronta).
//   4. Chama o escalonador para obter o mapeamento CPU → tarefa.
//   5. Aplica preempções e novas atribuições do escalonador.
//   6. Executa o tick (decrementa tempoRestante e quantumRestante).
//   7. Registra o estado de cada tarefa no histórico por tick do Gantt.
//   8. Salva o snapshot no historico[] para undo/redo.
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
    //    A tarefa volta à fila de prontos; o escalonador decidirá se ela continua
    //    ou se outra tarefa assume a CPU.
    for (auto& t : listaTarefas) {
        if (t.getEstadoAtual() == EstadoTarefa::Execucao && t.getQuantumRestante() == 0) {
            t.setEstadoAtual(EstadoTarefa::Pronta);
            for (auto& cpu : cpus)
                if (cpu.tarefaAtualID == t.getID())
                    cpu.tarefaAtualID = -1;
        }
    }

    // 3. Chegada de novas tarefas: Nova → Pronta ao atingir o instante de ingresso
    for (auto& t : listaTarefas)
        if (t.getEstadoAtual() == EstadoTarefa::Nova && t.getIngresso() <= T)
            t.setEstadoAtual(EstadoTarefa::Pronta);

    // 4. Chama o escalonador com o estado completo atual
    ResultadoEscalonamento res = pEscalonador->escalonar(listaTarefas, cpus, T);

    // 5. Aplica decisões do escalonador (preempções reais e novas atribuições).
    //    Para cada CPU: se a tarefa mudou, preempta a anterior (se ainda executando)
    //    e coloca a nova em execução com o quantum reiniciado.
    for (auto& [cpuId, tarefaId] : res.alocacao) {
        CPU* cpu = findCPU(cpuId);
        if (!cpu) continue;

        if (tarefaId == cpu->tarefaAtualID) {
            // Mesma tarefa: sem mudança necessária
            continue;
        }

        // Tarefa mudou: preempção da tarefa anterior (se havia uma em execução)
        if (cpu->tarefaAtualID != -1) {
            Tarefa* anterior = findTarefa(cpu->tarefaAtualID);
            if (anterior && anterior->getEstadoAtual() == EstadoTarefa::Execucao)
                anterior->setEstadoAtual(EstadoTarefa::Pronta);
        }

        cpu->tarefaAtualID = tarefaId;

        if (tarefaId == -1) {
            // CPU sem tarefa neste tick: desliga se não há nada pronto ou executando
            cpu->ligada = hasTarefaProntaOuExecutando();
        } else {
            Tarefa* nova = findTarefa(tarefaId);
            if (nova) {
                nova->setEstadoAtual(EstadoTarefa::Execucao);
                nova->setQuantumRestante(quantum);  // reinicia o quantum ao assumir a CPU
                cpu->ligada = true;
            }
        }
    }

    // 6. Executa o tick: decrementa contadores das tarefas em execução
    for (auto& t : listaTarefas) {
        if (t.getEstadoAtual() == EstadoTarefa::Execucao) {
            t.decrementarTempoRestante();
            t.decrementarQuantumRestante();
        }
    }

    // 7. Registra o estado de cada tarefa no histórico por tick (para o Gráfico de Gantt)
    for (auto& t : listaTarefas)
        t.registrarEstadoNoTempo(T, t.getEstadoAtual());

    // 8. Avança clock e salva snapshot no historico[] (inclui tarefas sorteadas para o Gantt)
    tickAtual = T;
    if (todasTerminadas()) simulacaoCompleta = true;
    historico.push_back(buildSnapshot(res.sorteadas));
}

// Restaura o estado completo do sistema (tarefas e CPUs) a partir de um snapshot.
// Garante que tickAtual, tempoRestante, quantumRestante e mapeamento CPU→tarefa
// sejam todos consistentes com o instante restaurado.
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

// Cria um snapshot do estado mutável atual para armazenar no histórico.
// 'sorteadas' vem do ResultadoEscalonamento deste tick (para o ícone ◆ no Gantt).
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

// ── Utilitários ──────────────────────────────────────────────────────────────

bool GerenciadorTarefa::todasTerminadas() const
{
    for (const auto& t : listaTarefas)
        if (t.getEstadoAtual() != EstadoTarefa::Terminada)
            return false;
    return !listaTarefas.empty();
}

// Retorna true quando há ao menos uma tarefa Pronta ou em Execucao.
// Intencionalmente NÃO inclui Nova: tarefas que ainda não chegaram
// não podem ser atribuídas a CPUs, logo a CPU deve ficar desligada
// até que ao menos uma tarefa esteja Pronta ou em Execucao.
bool GerenciadorTarefa::hasTarefaProntaOuExecutando() const
{
    for (const auto& t : listaTarefas) {
        auto s = t.getEstadoAtual();
        if (s == EstadoTarefa::Pronta || s == EstadoTarefa::Execucao)
            return true;
    }
    return false;
}

// Limite de segurança para evitar loop infinito em executarCompleto() caso
// haja um bug no escalonador. Usa a soma total das durações + maior ingresso + margem.
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

// Instancia o escalonador correspondente à string normalizada.
// Para adicionar um novo algoritmo: criar subclasse de Escalonador e
// adicionar um else-if aqui com a string identificadora. (req. 4.2)
Escalonador* GerenciadorTarefa::criarEscalonador(const std::string& tipo)
{
    if (tipo == "srtf")   return new SRTFEscalonador();
    return new PriopEscalonador();  // padrão: "priop"
}

// ── Getters ───────────────────────────────────────────────────────────────────

int                               GerenciadorTarefa::getTickAtual()  const { return tickAtual; }
int                               GerenciadorTarefa::getQuantum()    const { return quantum; }
int                               GerenciadorTarefa::getQtdeCpus()   const { return (int)cpus.size(); }
const std::vector<CPU>&           GerenciadorTarefa::getCPUs()       const { return cpus; }
const std::vector<Tarefa>&        GerenciadorTarefa::getTarefas()    const { return listaTarefas; }
const std::vector<EstadoSistema>& GerenciadorTarefa::getHistorico()  const { return historico; }
