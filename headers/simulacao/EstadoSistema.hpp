#pragma once
#include "tarefa/tarefa.hpp"
#include <map>
#include <vector>

// Snapshot do estado de uma tarefa em um instante — componente do EstadoSistema.
struct SnapshotTarefa {
    int          id;
    EstadoTarefa estado;
    int          tempoRestante;
    int          quantumRestante;
};

// Snapshot completo do sistema após um tick.
// O vetor GerenciadorTarefa::historico é uma sequência de EstadoSistema,
// onde historico[T] representa o estado do sistema após T ticks executados.
// Navegar pelo histórico (undo/redo) equivale a restaurar um desses snapshots.
struct EstadoSistema {
    int                         tempoClock;
    std::map<int, int>          alocacaoCPU;  // cpu_id → tarefa_id (-1 = sem tarefa)
    std::map<int, bool>         cpuLigada;    // cpu_id → está ligada?
    std::vector<SnapshotTarefa> tarefas;
    // IDs das tarefas cujo empate foi resolvido por sorteio neste tick (para ícone no Gantt)
    std::vector<int>            sorteadas;
};
