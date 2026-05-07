#include "escalonadores/SRTFEscalonador.hpp"
#include <set>

// Stub: para CPUs livres, escolhe a tarefa Pronta com menor tempo restante.
// Preempção real (SRTF é preemptivo) vem no passo 3.
std::map<int, int> SRTFEscalonador::escalonar(
    const std::vector<Tarefa>& tarefas,
    const std::vector<CPU>&    cpus,
    int /*tempoAtual*/)
{
    std::map<int, int> resultado;
    std::set<int> jaAtribuidos;

    std::vector<const Tarefa*> prontas;
    for (const auto& t : tarefas)
        if (t.getEstadoAtual() == EstadoTarefa::Pronta)
            prontas.push_back(&t);

    for (const auto& cpu : cpus) {
        if (cpu.tarefaAtualID != -1) {
            resultado[cpu.id] = cpu.tarefaAtualID;
            jaAtribuidos.insert(cpu.tarefaAtualID);
            continue;
        }

        resultado[cpu.id] = -1;

        const Tarefa* melhor = nullptr;
        for (const Tarefa* t : prontas) {
            if (jaAtribuidos.count(t->getID())) continue;
            if (melhor == nullptr
                || t->getTempoRestante() < melhor->getTempoRestante()
                || (t->getTempoRestante() == melhor->getTempoRestante()
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
