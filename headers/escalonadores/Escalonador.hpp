#pragma once
#include "tarefa/tarefa.hpp"
#include "simulacao/CPU.hpp"
#include <map>
#include <vector>

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
    //
    // Retorna: cpu_id → tarefa_id
    //   - mesmo tarefa_id que já estava: tarefa continua (sem context switch)
    //   - tarefa_id diferente: preempção + nova atribuição
    //   - -1: CPU fica sem tarefa neste tick
    virtual std::map<int, int> escalonar(
        const std::vector<Tarefa>& tarefas,
        const std::vector<CPU>&    cpus,
        int tempoAtual) = 0;
};
