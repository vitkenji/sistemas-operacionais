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
struct EstadoSistema {
    int                         tempoClock;
    std::map<int, int>          alocacaoCPU;  // qual tarefa esta em cada CPU
    std::map<int, bool>         cpuLigada;    // quais CPUs estao ligadas
    std::vector<SnapshotTarefa> tarefas; // o snapshot atual de cada tarefa
    std::vector<int>            sorteadas; //quais tarefas tiveram empate resolvido por sorteio nesse tick
};
