#pragma once
#include "tarefa/tarefa.hpp"
#include "simulacao/CPU.hpp"
#include <map>
#include <vector>

// Resultado devolvido por Escalonador::escalonar().
struct ResultadoEscalonamento {
    // cpu_id → tarefa_id (-1 = CPU fica sem tarefa neste tick)
    // tarefa_id diferente do atual indica preempção da tarefa anterior
    std::map<int, int> alocacao;

    // IDs das tarefas cujo empate foi resolvido por sorteio neste tick.
    // Usado para exibir ícone no Gráfico de Gantt (req. 4.3 item 4).
    std::vector<int> sorteadas;
};

class Escalonador {
public:
    virtual ~Escalonador() = default;

    // Decide o mapeamento completo CPU → tarefa para o tick atual.
    // O motor chama isto após tratar chegadas e finalizações,
    // mas ANTES de decrementar os contadores.
    //
    // tarefas:    todas as tarefas do sistema (com estadoAtual, tempoRestante, etc.)
    // cpus:       estado atual das CPUs (incluindo as que já têm tarefa rodando)
    // tempoAtual: tick corrente
    virtual ResultadoEscalonamento escalonar(
        const std::vector<Tarefa>& tarefas,
        const std::vector<CPU>&    cpus,
        int tempoAtual) = 0;
};
