#include "escalonadores/PriopDEscalonador.hpp"
#include <algorithm>
#include <map>
#include <random>
#include <set>

static constexpr int ALPHA_ENVELHECIMENTO = 1;

// gerador de numeros aleatórios
static std::mt19937& rng() {
    static std::mt19937 inst(std::random_device{}());
    return inst;
}

// true se a é preferível a b nos critérios de PRIOpD.
// desempate: prioridade dinâmica maior -> em execução -> menor ingresso -> menor duração.
static bool melhorPriopD(const Tarefa* a, const Tarefa* b)
{
    if (a->getPrioridadeDinamica() != b->getPrioridadeDinamica())
        return a->getPrioridadeDinamica() > b->getPrioridadeDinamica();

    bool aEx = a->getEstadoAtual() == EstadoTarefa::Execucao;
    bool bEx = b->getEstadoAtual() == EstadoTarefa::Execucao;
    if (aEx != bEx) return aEx;

    if (a->getIngresso() != b->getIngresso())
        return a->getIngresso() < b->getIngresso();

    return a->getDuracao() < b->getDuracao();
}

// true se a e b empatam em todos os critérios
static bool empatePriopD(const Tarefa* a, const Tarefa* b)
{
    bool aEx = a->getEstadoAtual() == EstadoTarefa::Execucao;
    bool bEx = b->getEstadoAtual() == EstadoTarefa::Execucao;
    return a->getPrioridadeDinamica() == b->getPrioridadeDinamica()
        && aEx == bEx
        && a->getIngresso() == b->getIngresso()
        && a->getDuracao()  == b->getDuracao();
}

ResultadoEscalonamento PriopDEscalonador::escalonar(
    std::vector<Tarefa>&       tarefas,
    const std::vector<CPU>&    cpus,
    int /*tempoAtual*/)
{
    ResultadoEscalonamento res;
    int N = (int)cpus.size();

    // candidatas: tarefas pronta ou em execução
    std::vector<Tarefa*> candidatas;
    for (auto& t : tarefas)
        if (t.getEstadoAtual() == EstadoTarefa::Pronta ||
            t.getEstadoAtual() == EstadoTarefa::Execucao)
            candidatas.push_back(&t);

    if (candidatas.empty()) {
        for (const auto& cpu : cpus)
            res.alocacao[cpu.id] = "";
        return res;
    }

    // sorteia para desempate.
    std::uniform_int_distribution<int> dist(0, 1'000'000);
    std::map<std::string, int> aleatorio;
    for (const Tarefa* t : candidatas)
        aleatorio[t->getID()] = dist(rng());

    // ordena as candidatas pelo PRIOpD
    std::sort(candidatas.begin(), candidatas.end(),
        [&](const Tarefa* a, const Tarefa* b) {
            if (!empatePriopD(a, b)) return melhorPriopD(a, b);
            if (aleatorio[a->getID()] != aleatorio[b->getID()])
                return aleatorio[a->getID()] < aleatorio[b->getID()];
            return a->getID() < b->getID();
        });

    // seleciona as N melhores candidatas (N = número de CPUs disponíveis)
    int qtde = std::min(N, (int)candidatas.size());

    // Detecta sorteio. ID é repassado ao grafico para exibir icone de sorteio.
    if (qtde > 0 && qtde < (int)candidatas.size()) {
        if (empatePriopD(candidatas[qtde - 1], candidatas[qtde]))
            res.sorteadas.push_back(candidatas[qtde - 1]->getID());
    }

    // qual tarefa está rodando em qual CPU atualmente
    std::map<std::string, int> tarefaParaCPU;
    for (const auto& cpu : cpus)
        if (!cpu.tarefaAtualID.empty())
            tarefaParaCPU[cpu.tarefaAtualID] = cpu.id;

    std::set<int> cpusUsados;
    std::set<std::string> tarefasAlocadas;

    // mantém no mesmo CPU as tarefas selecionadas que já estavam rodando.
    // evita context switch quando não há motivo para trocar de CPU.
    for (int i = 0; i < qtde; ++i) {
        std::string tid = candidatas[i]->getID();
        auto it = tarefaParaCPU.find(tid);
        if (it != tarefaParaCPU.end()) {
            res.alocacao[it->second] = tid;
            cpusUsados.insert(it->second);
            tarefasAlocadas.insert(tid);
        }
    }

    // distribui as tarefas restantes às CPUs que ficaram sem atribuição
    std::vector<int> cpusLivres;
    for (const auto& cpu : cpus)
        if (!cpusUsados.count(cpu.id))
            cpusLivres.push_back(cpu.id);

    int idx = 0;
    for (int i = 0; i < qtde && idx < (int)cpusLivres.size(); ++i) {
        std::string tid = candidatas[i]->getID();
        if (!tarefasAlocadas.count(tid)) {
            res.alocacao[cpusLivres[idx++]] = tid;
            tarefasAlocadas.insert(tid);
        }
    }

    // CPUs que sobraram ficam ociosas
    while (idx < (int)cpusLivres.size())
        res.alocacao[cpusLivres[idx++]] = "";

    for (auto& t : tarefas) {
        if (tarefasAlocadas.count(t.getID())) {
            t.resetarPrioridadeDinamica();
        } else if (t.getEstadoAtual() == EstadoTarefa::Pronta) {
            t.incrementarPrioridadeDinamica(ALPHA_ENVELHECIMENTO);
        }
    }

    return res;
}
