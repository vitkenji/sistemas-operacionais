#pragma once
// Escalonador.hpp
// Interface abstrata do escalonador — padrão Strategy (req. 4.2).
//
// Decisão de design: o escalonador é definido como uma classe virtual pura
// para que novos algoritmos possam ser adicionados sem modificar o motor
// de simulação (GerenciadorTarefa). Para incluir um novo algoritmo basta:
//   1. Criar uma subclasse de Escalonador em headers/escalonadores/ e src/escalonadores/
//   2. Adicionar um else-if em GerenciadorTarefa::criarEscalonador()
//
// O contrato do método escalonar():
//   - Recebe o estado ATUAL do sistema (tarefas e CPUs) APÓS chegadas e finalizações,
//     mas ANTES de decrementar contadores.
//   - Retorna ResultadoEscalonamento: um mapeamento completo cpu_id → tarefa_id
//     para o tick atual, e a lista de tarefas cujo empate foi resolvido por sorteio.
//   - O motor aplica as decisões (preemptando tarefas removidas e colocando as novas).

#include "tarefa/tarefa.hpp"
#include "simulacao/CPU.hpp"
#include <map>
#include <vector>

// Resultado devolvido por Escalonador::escalonar() para cada tick.
struct ResultadoEscalonamento {
    // Mapeamento cpu_id → tarefa_id para este tick.
    // tarefa_id == -1 significa que a CPU ficará ociosa (sem tarefa disponível).
    std::map<int, int> alocacao;

    // IDs de todas as tarefas do grupo de empate que cruzou a fronteira
    // selecionado/excluído neste tick. O GanttChart exibe o ícone ◆ sobre elas.
    std::vector<int> sorteadas;
};

class Escalonador {
public:
    virtual ~Escalonador() = default;

    // Decide o mapeamento CPU → tarefa para o tick atual.
    // Parâmetros:
    //   tarefas    — todas as tarefas do sistema (estadoAtual, tempoRestante, etc.)
    //   cpus       — estado atual das CPUs (incluindo qual tarefa já está rodando)
    //   tempoAtual — tick corrente (raramente necessário, mas disponível)
    virtual ResultadoEscalonamento escalonar(
        const std::vector<Tarefa>& tarefas,
        const std::vector<CPU>&    cpus,
        int tempoAtual) = 0;
};
