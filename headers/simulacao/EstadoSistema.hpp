#pragma once
#include "simulacao/Mutex.hpp"
#include "tarefa/tarefa.hpp"
#include <cstddef>
#include <map>
#include <string>
#include <vector>

enum class TipoEventoGantt {
    SolicitarMutex,
    LiberarMutex
};

struct EventoGantt {
    std::string tarefaId;
    TipoEventoGantt tipo;
    int mutexId;
};

// snapshot do estado de uma tarefa em um instante
struct SnapshotTarefa {
    std::string  id;
    EstadoTarefa estado;
    MotivoSuspensao motivoSuspensao;
    int          tempoRestante;
    int          quantumRestante;
    int          prioridadeDinamica;
    std::size_t  proximaAcaoIndex;
};

struct SnapshotMutex {
    int id;
    std::string donoTarefaID;
    std::vector<std::string> filaEspera;
};

// snapshot do sistema depois de um tick.
struct EstadoSistema {
    int                              tempoClock;
    std::map<int, std::string>       alocacaoCPU;  // cpu_id -> tarefa_id
    std::map<int, bool>              cpuLigada;
    std::vector<SnapshotTarefa>      tarefas;
    std::vector<SnapshotMutex>       mutexes;
    std::vector<std::string>         sorteadas;
    std::vector<EventoGantt>         eventos;
};
