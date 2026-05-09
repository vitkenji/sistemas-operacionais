#pragma once
// EstadoSistema.hpp
// Snapshot completo do sistema em um determinado tick — usado para undo/redo (req. 1.5.2).
//
// Decisão de design: em vez de guardar snapshots de objetos completos (pesados),
// guardamos apenas os campos mutáveis de cada tarefa (estado, tempos) e o mapeamento
// de CPUs. Os parâmetros fixos (id, ingresso, duração, prioridade) nunca mudam e
// não precisam ser copiados a cada tick.
//
// O vetor 'historico' no GerenciadorTarefa armazena um EstadoSistema por tick,
// possibilitando avançar/retroceder a simulação em O(1) por tick.

#include "tarefa/tarefa.hpp"
#include <map>
#include <vector>

// Snapshot dos campos mutáveis de uma tarefa em um instante específico.
struct SnapshotTarefa {
    int          id;
    EstadoTarefa estado;
    int          tempoRestante;
    int          quantumRestante;
};

// Snapshot completo do sistema após um tick.
struct EstadoSistema {
    int                         tempoClock;   // tick correspondente a este snapshot
    std::map<int, int>          alocacaoCPU;  // cpu_id → tarefa_id (-1 = sem tarefa)
    std::map<int, bool>         cpuLigada;    // cpu_id → está ligada?
    std::vector<SnapshotTarefa> tarefas;      // estado de cada tarefa neste tick
    // IDs das tarefas cujo empate foi resolvido por sorteio neste tick.
    // Usado pelo GanttChart para exibir o ícone ◆ (req. 4.3 item 4).
    std::vector<int>            sorteadas;
};
