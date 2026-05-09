#include "escalonadores/PriopEscalonador.hpp"
#include <algorithm>
#include <map>
#include <random>
#include <set>

// Gerador de números aleatórios compartilhado (semeado uma vez na inicialização)
static std::mt19937& rng() {
    static std::mt19937 inst(std::random_device{}());
    return inst;
}

// Retorna true se 'a' é preferível a 'b' nos critérios determinísticos de PRIOp.
// Ordem: prioridade DESC → em execução primeiro → ingresso ASC → duração ASC.
// Não inclui sorteio — isso é detectado separadamente.
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

// Retorna true se 'a' e 'b' são deterministicamente iguais (empate → sorteio).
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

    // Candidatas: tarefas que podem ocupar uma CPU (Pronta ou já em Execucao)
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

    // Chave aleatória por tarefa para desempate final (sorteio — req. 4.3 item 4)
    std::uniform_int_distribution<int> dist(0, 1'000'000);
    std::map<int, int> aleatorio;
    for (const Tarefa* t : candidatas)
        aleatorio[t->getID()] = dist(rng());

    // Ordena pelas regras de PRIOp + chave aleatória como desempate final
    std::sort(candidatas.begin(), candidatas.end(),
        [&](const Tarefa* a, const Tarefa* b) {
            if (!empatePriop(a, b)) return melhorPriop(a, b);
            // Empate determinístico: sorteio decide
            if (aleatorio[a->getID()] != aleatorio[b->getID()])
                return aleatorio[a->getID()] < aleatorio[b->getID()];
            return a->getID() < b->getID();  // desempate absoluto por ID (evita violação de strict weak order)
        });

    int qtde = std::min(N, (int)candidatas.size());

    // Detecta sorteio na fronteira (última selecionada vs. primeira excluída)
    if (qtde > 0 && qtde < (int)candidatas.size()) {
        if (empatePriop(candidatas[qtde - 1], candidatas[qtde]))
            res.sorteadas.push_back(candidatas[qtde - 1]->getID());
    }

    // ── Fase de atribuição ───────────────────────────────────────────────────
    // Mapeamento inverso: tarefa_id → cpu_id (para tarefas atualmente em execução)
    std::map<int, int> tarefaParaCPU;
    for (const auto& cpu : cpus)
        if (cpu.tarefaAtualID != -1)
            tarefaParaCPU[cpu.tarefaAtualID] = cpu.id;

    std::set<int> cpusUsados;
    std::set<int> tarefasAlocadas;

    // 1ª passagem: mantém no mesmo CPU as tarefas selecionadas que já estavam rodando
    //   (evita context switch desnecessário — req. 4.3 item 1)
    for (int i = 0; i < qtde; ++i) {
        int tid = candidatas[i]->getID();
        auto it = tarefaParaCPU.find(tid);
        if (it != tarefaParaCPU.end()) {
            res.alocacao[it->second] = tid;
            cpusUsados.insert(it->second);
            tarefasAlocadas.insert(tid);
        }
    }

    // 2ª passagem: atribui tarefas restantes às CPUs que ficaram livres
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

    // CPUs que sobraram ficam sem tarefa neste tick
    while (idx < (int)cpusLivres.size())
        res.alocacao[cpusLivres[idx++]] = -1;

    return res;
}
