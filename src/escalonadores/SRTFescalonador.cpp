#include "escalonadores/SRTFEscalonador.hpp"
#include <algorithm>
#include <map>
#include <random>
#include <set>

// gerador de numeros aleatorios
static std::mt19937& rng() {
    static std::mt19937 inst(std::random_device{}());
    return inst;
}

// true se a é preferível a b no SRTF.
// desempate: menor tempo restante -> já em execução -> menor ingresso -> menor duração
static bool melhorSRTF(const Tarefa* a, const Tarefa* b)
{
    if (a->getTempoRestante() != b->getTempoRestante())
        return a->getTempoRestante() < b->getTempoRestante();

    bool aEx = a->getEstadoAtual() == EstadoTarefa::Execucao;
    bool bEx = b->getEstadoAtual() == EstadoTarefa::Execucao;
    if (aEx != bEx) return aEx;

    if (a->getIngresso() != b->getIngresso())
        return a->getIngresso() < b->getIngresso();

    return a->getDuracao() < b->getDuracao();
}

// true se a e b empatam em todos os criterios.
// assim, o desempate é feito por sorteio.
static bool empateSRTF(const Tarefa* a, const Tarefa* b)
{
    bool aEx = a->getEstadoAtual() == EstadoTarefa::Execucao;
    bool bEx = b->getEstadoAtual() == EstadoTarefa::Execucao;
    return a->getTempoRestante() == b->getTempoRestante()
        && aEx == bEx
        && a->getIngresso() == b->getIngresso()
        && a->getDuracao()  == b->getDuracao();
}

ResultadoEscalonamento SRTFEscalonador::escalonar(
    const std::vector<Tarefa>& tarefas,
    const std::vector<CPU>&    cpus,
    int /*tempoAtual*/)
{
    ResultadoEscalonamento res;
    int N = (int)cpus.size();

    // candidatas sao tarefas em pronta ou em execucao
    // novas, suspensas e terminadas são ignoradas
    std::vector<const Tarefa*> candidatas;
    for (const auto& t : tarefas)
        if (t.getEstadoAtual() == EstadoTarefa::Pronta ||
            t.getEstadoAtual() == EstadoTarefa::Execucao)
            candidatas.push_back(&t);

    if (candidatas.empty()) {
        for (const auto& cpu : cpus)
            res.alocacao[cpu.id] = -1;
        return res;
    }

    // sorteia
    std::uniform_int_distribution<int> dist(0, 1'000'000);
    std::map<int, int> aleatorio;
    for (const Tarefa* t : candidatas)
        aleatorio[t->getID()] = dist(rng());

    // ordena as candidatas pelo SRTF
    std::sort(candidatas.begin(), candidatas.end(),
        [&](const Tarefa* a, const Tarefa* b) {
            if (!empateSRTF(a, b)) return melhorSRTF(a, b);
            if (aleatorio[a->getID()] != aleatorio[b->getID()])
                return aleatorio[a->getID()] < aleatorio[b->getID()];
            return a->getID() < b->getID();
        });

    // seleciona as N candidatas (N = número de CPUs disponíveis)
    int qtde = std::min(N, (int)candidatas.size());

    // detecta sorteio. ID é repassado ao Gráfico de Gantt para exibir o ícone de sorteio.
    if (qtde > 0 && qtde < (int)candidatas.size()) {
        if (empateSRTF(candidatas[qtde - 1], candidatas[qtde]))
            res.sorteadas.push_back(candidatas[qtde - 1]->getID());
    }

    // qual tarefa está rodando em qual CPU atualmente
    std::map<int, int> tarefaParaCPU;
    for (const auto& cpu : cpus)
        if (cpu.tarefaAtualID != -1)
            tarefaParaCPU[cpu.tarefaAtualID] = cpu.id;

    std::set<int> cpusUsados;
    std::set<int> tarefasAlocadas;

    // mantém no mesmo CPU as tarefas selecionadas que já estavam rodando.
    // Isso evita troca de contexto quando não há motivo para trocar de CPU.
    for (int i = 0; i < qtde; ++i) {
        int tid = candidatas[i]->getID();
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
        int tid = candidatas[i]->getID();
        if (!tarefasAlocadas.count(tid)) {
            res.alocacao[cpusLivres[idx++]] = tid;
            tarefasAlocadas.insert(tid);
        }
    }

    // CPUs que sobraram ficam ociosas 
    while (idx < (int)cpusLivres.size())
        res.alocacao[cpusLivres[idx++]] = -1;

    return res;
}
