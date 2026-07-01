#pragma once
#include <map>
#include <cstddef>
#include <string>
#include <vector>

// estados possíveis de uma tarefa ao longo do seu ciclo de vida
// nova -> pronta ocorre quando o tick atinge o instante de ingresso
// execucao -> pronta pode ocorrer por preempção
// execucao -> terminada ocorre quando tempoRestante chega a zero
enum class EstadoTarefa {
    Nova,
    Pronta,
    Execucao,
    Suspensa,
    Terminada
};

enum class TipoAcaoTarefa {
    SolicitarMutex,
    LiberarMutex,
    EntradaSaida
};

enum class MotivoSuspensao {
    Nenhum,
    Manual,
    Mutex,
    EntradaSaida
};

struct AcaoTarefa {
    TipoAcaoTarefa tipo;
    int mutexId;
    int tempoRelativo;
    int duracaoIO;
};

// TCB: representa uma tarefa no simulador.
// armazena atributos lidos do arquivo, estado corrente e o histórico
class Tarefa {
private:
    // atributos definidos no txt, nunca mudam
    std::string  ID;
    std::string  corHex;
    int          ingresso;   
    int          duracao; 
    int          prioridade;
    int          prioridadeDinamica;
    std::vector<AcaoTarefa> acoes;

    EstadoTarefa estadoAtual;
    MotivoSuspensao motivoSuspensao;
    int          tempoRestante;  
    int          quantumRestante; // 0 quando não está em execução
    std::size_t  proximaAcaoIndex;

    // histórico por tick: map<tick, estado> alimenta o Gráfico de Gantt.
    // usar map com sobrescrita permite que undo/redo reescreva ticks já calculados
    // sem precisar limpar o histórico manualmente
    std::map<int, EstadoTarefa> historicoNoTempo;

public:
    Tarefa(std::string id, std::string corHex, int ingresso, int duracao,
           int prioridade, std::vector<AcaoTarefa> acoes);
    ~Tarefa();

    std::string getID()         const;
    std::string getCorHex()     const;
    int         getIngresso()   const;
    int         getDuracao()    const;
    int         getPrioridade() const;
    int         getPrioridadeDinamica() const;
    int         getTempoExecutado() const;
    const std::vector<AcaoTarefa>& getAcoes() const;
    std::size_t getProximaAcaoIndex() const;
    MotivoSuspensao getMotivoSuspensao() const;

    EstadoTarefa getEstadoAtual()    const;
    int          getTempoRestante()   const;
    int          getQuantumRestante() const;

    void setEstadoAtual(EstadoTarefa estado);
    void setTempoRestante(int t);
    void setQuantumRestante(int q);
    void setPrioridadeDinamica(int p);
    void setProximaAcaoIndex(std::size_t index);
    void setMotivoSuspensao(MotivoSuspensao motivo);
    void resetarPrioridadeDinamica();
    void incrementarPrioridadeDinamica(int incremento);
    void avancarAcao();
    void decrementarTempoRestante();
    void decrementarQuantumRestante();

    // histórico por tick
    void         registrarEstadoNoTempo(int tick, EstadoTarefa estado);
    EstadoTarefa buscarEstadoNoTempo(int tick) const;
};
