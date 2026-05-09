// SRTFescalonador.cpp
// Implementação do escalonador SRTF (Shortest Remaining Time First).
//
// Fluxo do algoritmo a cada tick:
//   1. Coleta candidatas: tarefas em estado Pronta ou Execucao.
//   2. Ordena por tempo restante crescente; empates resolvidos pelos critérios do req. 4.3.
//   3. Sorteia aleatoriamente entre as que empatam deterministicamente na fronteira
//      selecionado/excluído e registra o grupo para exibição do ícone ◆ no Gantt.
//   4. Distribui as N melhores pelas N CPUs, preservando o vínculo tarefa↔CPU atual
//      quando possível (evita troca de contexto desnecessária — req. 4.3 item 1).
//
// Preempção: o SRTF pode preemptar uma tarefa em execução se uma tarefa com
// tempo restante estritamente menor chegar no mesmo tick (a nova tarefa terá
// posição superior na ordenação). O quantum é gerenciado pelo motor
// (GerenciadorTarefa) e serve como limite superior ao tempo de uso contínuo.

#include "escalonadores/SRTFEscalonador.hpp"
#include <algorithm>
#include <map>
#include <random>
#include <set>

// Gerador de números aleatórios com semente aleatória — semeado uma única vez
// na primeira chamada. Compartilhado entre todas as invocações do escalonador.
static std::mt19937& rng() {
    static std::mt19937 inst(std::random_device{}());
    return inst;
}

// Retorna true se 'a' é estritamente preferível a 'b' pelos critérios determinísticos do SRTF.
// Critérios em ordem (req. 4.3):
//   1. Tempo restante menor (menor = mais prioritário no SRTF)
//   2. Já em execução (evita context switch)
//   3. Ingresso menor (chegou antes)
//   4. Duração menor
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

// Retorna true se 'a' e 'b' empatam em TODOS os critérios determinísticos.
// Quando isso ocorre, o sorteio decide e o ícone ◆ aparece no Gantt (req. 4.3 item 4).
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

    // 1. Coleta tarefas que podem receber CPU (Pronta ou já em Execucao)
    std::vector<const Tarefa*> candidatas;
    for (const auto& t : tarefas)
        if (t.getEstadoAtual() == EstadoTarefa::Pronta ||
            t.getEstadoAtual() == EstadoTarefa::Execucao)
            candidatas.push_back(&t);

    // Sem candidatas: todas as CPUs ficam ociosas/desligadas neste tick
    if (candidatas.empty()) {
        for (const auto& cpu : cpus)
            res.alocacao[cpu.id] = -1;
        return res;
    }

    // 2. Atribui chave aleatória a cada candidata para resolver empates por sorteio (req. 4.3 item 4)
    std::uniform_int_distribution<int> dist(0, 1'000'000);
    std::map<int, int> aleatorio;
    for (const Tarefa* t : candidatas)
        aleatorio[t->getID()] = dist(rng());

    // Ordena pelos critérios determinísticos; empate total → sorteio via chave aleatória;
    // empate de chave (improvável mas possível) → ID como tiebreaker absoluto para
    // garantir strict weak order e evitar comportamento indefinido no std::sort.
    std::sort(candidatas.begin(), candidatas.end(),
        [&](const Tarefa* a, const Tarefa* b) {
            if (!empateSRTF(a, b)) return melhorSRTF(a, b);
            if (aleatorio[a->getID()] != aleatorio[b->getID()])
                return aleatorio[a->getID()] < aleatorio[b->getID()];
            return a->getID() < b->getID();
        });

    int qtde = std::min(N, (int)candidatas.size());

    // 3. Detecta todas as tarefas envolvidas em sorteio.
    // Um sorteio ocorre quando a última tarefa selecionada empata deterministicamente
    // com a primeira excluída — ou seja, a loteria determinou quem ficou de fora.
    // Nesse caso marcamos TODOS os membros do grupo de empate (selecionados e excluídos),
    // pois todas as suas posições foram decididas pelo sorteio.
    if (qtde > 0 && qtde < (int)candidatas.size()) {
        const Tarefa* fronteira = candidatas[qtde];  // primeira excluída
        if (empateSRTF(candidatas[qtde - 1], fronteira)) {
            // Scan para trás: selecionadas que pertencem ao mesmo grupo
            for (int i = qtde - 1; i >= 0 && empateSRTF(candidatas[i], fronteira); --i)
                res.sorteadas.push_back(candidatas[i]->getID());
            // Scan para frente: excluídas do mesmo grupo (incluindo 'fronteira')
            for (int i = qtde; i < (int)candidatas.size() && empateSRTF(candidatas[i], fronteira); ++i)
                res.sorteadas.push_back(candidatas[i]->getID());
        }
    }

    // 4. Distribui as N melhores candidatas pelas CPUs.
    //    Estratégia em duas passagens para minimizar trocas de contexto (req. 4.3 item 1):
    //    Passagem 1: mantém no mesmo CPU as tarefas selecionadas que já estavam rodando.
    //    Passagem 2: atribui as tarefas novas às CPUs que sobraram.
    std::map<int, int> tarefaParaCPU;  // tarefa_id → cpu_id atual
    for (const auto& cpu : cpus)
        if (cpu.tarefaAtualID != -1)
            tarefaParaCPU[cpu.tarefaAtualID] = cpu.id;

    std::set<int> cpusUsados;
    std::set<int> tarefasAlocadas;

    // Passagem 1: preserva vínculo existente
    for (int i = 0; i < qtde; ++i) {
        int tid = candidatas[i]->getID();
        auto it = tarefaParaCPU.find(tid);
        if (it != tarefaParaCPU.end()) {
            res.alocacao[it->second] = tid;
            cpusUsados.insert(it->second);
            tarefasAlocadas.insert(tid);
        }
    }

    // Passagem 2: novas tarefas assumem CPUs livres (na ordem em que aparecem na lista)
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

    // CPUs sem candidata ficam ociosas/desligadas neste tick
    while (idx < (int)cpusLivres.size())
        res.alocacao[cpusLivres[idx++]] = -1;

    return res;
}
