#include "escalonadores/PriopEscalonador.hpp"
#include <algorithm>
#include <map>
#include <random>
#include <set>

// gerador de numeros aleatórios
static std::mt19937& rng() {
    static std::mt19937 inst(std::random_device{}());
    return inst;
}

// true se a é preferível a b nos critérios de PRIOp.
// desempate: prioridade maior -> em execução -> menor ingresso → menor duração.
static bool melhorPriop(const Tarefa* a, const Tarefa* b)
{
    if (a->getPrioridade() != b->getPrioridade())
        return a->getPrioridade() > b->getPrioridade();

    bool aEx = a->getEstadoAtual() == EstadoTarefa::Execucao;
    bool bEx = b->getEstadoAtual() == EstadoTarefa::Execucao;
    if (aEx != bEx) return aEx;

    if (a->getIngresso() != b->getIngresso())
        return a->getIngresso() < b->getIngresso();

    return a->getDuracao() < b->getDuracao();
}

// Retorna true se 'a' e 'b' empatam em todos os critérios determinísticos.
// Quando há empate, o desempate final é feito por sorteio (número aleatório).
static bool empatePriop(const Tarefa* a, const Tarefa* b)
{
    bool aEx = a->getEstadoAtual() == EstadoTarefa::Execucao;
    bool bEx = b->getEstadoAtual() == EstadoTarefa::Execucao;
    return a->getPrioridade() == b->getPrioridade()
        && aEx == bEx
        && a->getIngresso() == b->getIngresso()
        && a->getDuracao()  == b->getDuracao();
}

ResultadoEscalonamento PriopEscalonador::escalonar(
    const std::vector<Tarefa>& tarefas,
    const std::vector<CPU>&    cpus,
    int /*tempoAtual*/)
{
    ResultadoEscalonamento res;
    int N = (int)cpus.size();

    // Candidatas são tarefas que podem rodar neste tick: Pronta ou já em Execução.
    // Nova, Suspensa e Terminada ignoradas.
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

    // sorteia chave aleatória por tarefa para desempate.
    // Usar chaves geradas antes da ordenação garante consistência dentro do tick.
    std::uniform_int_distribution<int> dist(0, 1'000'000);
    std::map<int, int> aleatorio;
    for (const Tarefa* t : candidatas)
        aleatorio[t->getID()] = dist(rng());

    // Ordena as candidatas pelas regras do PRIOp.
    // O ID é usado como desempate absoluto final para garantir strict weak order,
    // exigido pelo std::sort (sem ele, ordenações com chaves aleatórias iguais
    // poderiam produzir comportamento indefinido).
    std::sort(candidatas.begin(), candidatas.end(),
        [&](const Tarefa* a, const Tarefa* b) {
            if (!empatePriop(a, b)) return melhorPriop(a, b);
            if (aleatorio[a->getID()] != aleatorio[b->getID()])
                return aleatorio[a->getID()] < aleatorio[b->getID()];
            return a->getID() < b->getID();
        });

    // seleciona as N melhores candidatas (N = número de CPUs disponíveis)
    int qtde = std::min(N, (int)candidatas.size());

    // Detecta sorteio na fronteira: se a última tarefa selecionada e a primeira
    // excluída empatam deterministicamente, o resultado foi decidido por sorteio.
    // Esse ID é repassado ao Gráfico de Gantt para exibir o ícone de sorteio.
    if (qtde > 0 && qtde < (int)candidatas.size()) {
        if (empatePriop(candidatas[qtde - 1], candidatas[qtde]))
            res.sorteadas.push_back(candidatas[qtde - 1]->getID());
    }

    // fase de atribuição: duas passagens para minimizar context switches

    // mapeamento inverso: qual tarefa está rodando em qual CPU atualmente
    std::map<int, int> tarefaParaCPU;
    for (const auto& cpu : cpus)
        if (cpu.tarefaAtualID != -1)
            tarefaParaCPU[cpu.tarefaAtualID] = cpu.id;

    std::set<int> cpusUsados;
    std::set<int> tarefasAlocadas;

    // 1ª passagem: mantém no mesmo CPU as tarefas selecionadas que já estavam rodando.
    // Isso evita context switch quando não há motivo para trocar de CPU.
    for (int i = 0; i < qtde; ++i) {
        int tid = candidatas[i]->getID();
        auto it = tarefaParaCPU.find(tid);
        if (it != tarefaParaCPU.end()) {
            res.alocacao[it->second] = tid;
            cpusUsados.insert(it->second);
            tarefasAlocadas.insert(tid);
        }
    }

    // 2ª passagem: distribui as tarefas restantes às CPUs que ficaram sem atribuição
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

    // CPUs que sobraram ficam ociosas neste tick
    while (idx < (int)cpusLivres.size())
        res.alocacao[cpusLivres[idx++]] = -1;

    return res;
}
