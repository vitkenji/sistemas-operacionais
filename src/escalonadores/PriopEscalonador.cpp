#include "escalonadores/PriopEscalonador.hpp"
#include <set>

// Critérios de desempate (passo 3 implementa a lógica completa de PRIOp):
// Para CPUs livres: atribui a tarefa Pronta com maior prioridade.
// Não preempta tarefas em execução — a preempção voluntária vem no passo 3.
std::map<int, int> PriopEscalonador::escalonar(
    const std::vector<Tarefa>& tarefas,
    const std::vector<CPU>&    cpus,
    int /*tempoAtual*/)
{
    std::map<int, int> resultado;
    std::set<int> jaAtribuidos;

    // Tarefas elegíveis (Pronta)
    std::vector<const Tarefa*> prontas;
    for (const auto& t : tarefas)
        if (t.getEstadoAtual() == EstadoTarefa::Pronta)
            prontas.push_back(&t);

    for (const auto& cpu : cpus) {
        if (cpu.tarefaAtualID != -1) {
            // CPU já ocupada: mantém a tarefa atual (sem preempção voluntária neste stub)
            resultado[cpu.id] = cpu.tarefaAtualID;
            jaAtribuidos.insert(cpu.tarefaAtualID);
            continue;
        }

        // CPU livre: escolhe melhor tarefa pronta
        resultado[cpu.id] = -1;

        const Tarefa* melhor = nullptr;
        for (const Tarefa* t : prontas) {
            if (jaAtribuidos.count(t->getID())) continue;
            if (melhor == nullptr
                || t->getPrioridade() > melhor->getPrioridade()
                || (t->getPrioridade() == melhor->getPrioridade()
                    && t->getIngresso() < melhor->getIngresso()))
            {
                melhor = t;
            }
        }

        if (melhor) {
            resultado[cpu.id] = melhor->getID();
            jaAtribuidos.insert(melhor->getID());
        }
    }

    return resultado;
}
