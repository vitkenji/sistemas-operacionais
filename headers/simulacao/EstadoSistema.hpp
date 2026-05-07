#pragma once
#include "tarefa/tarefa.hpp"
#include <map>
#include <vector>

// Snapshot do estado de uma tarefa em um instante — usado para undo/redo
struct SnapshotTarefa {
    int          id;
    EstadoTarefa estado;
    int          tempoRestante;
    int          quantumRestante;
};

// Snapshot completo do sistema após um tick — armazenado no histórico
struct EstadoSistema {
    int                         tempoClock;
    std::map<int, int>          alocacaoCPU;  // cpu_id → tarefa_id (-1 = sem tarefa)
    std::map<int, bool>         cpuLigada;    // cpu_id → está ligada?
    std::vector<SnapshotTarefa> tarefas;
};
