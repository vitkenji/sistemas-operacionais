#pragma once
#include "tarefa/tarefa.hpp"
#include <map>
#include <vector>

// snapshot do estado de uma tarefa em um instante
struct SnapshotTarefa {
    int          id;
    EstadoTarefa estado;
    int          tempoRestante;
    int          quantumRestante;
};

// snapshot do sistema depois de um tick.
// GerenciadorTarefa::historico é uma sequência de EstadoSistema
// historico[T] representa  estado do sistema após T ticks
// undo/redo equivale a restaurar um desses snapshots.
struct EstadoSistema {
    int                         tempoClock;
    std::map<int, int>          alocacaoCPU;  // cpu_id -> tarefa_id (-1 = sem tarefa)
    std::map<int, bool>         cpuLigada;    // cpu_id -> está ligada?
    std::vector<SnapshotTarefa> tarefas;
    // IDs das tarefas cujo empate foi resolvido por sorteio
    std::vector<int>            sorteadas;
};
