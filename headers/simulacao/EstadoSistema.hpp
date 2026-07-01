#pragma once
#include "tarefa/tarefa.hpp"
#include <map>
#include <vector>

// snapshot do estado de uma tarefa em um instante
struct SnapshotTarefa {
    std::string  id;
    EstadoTarefa estado;
    int          tempoRestante;
    int          quantumRestante;
    int          prioridadeDinamica;
};

// snapshot do sistema depois de um tick.
struct EstadoSistema {
    int                              tempoClock;
    std::map<int, std::string>       alocacaoCPU;  // cpu_id -> tarefa_id
    std::map<int, bool>              cpuLigada;
    std::vector<SnapshotTarefa>      tarefas;
    std::vector<std::string>         sorteadas;
};
