#pragma once
#include <map>
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
    std::vector<int> lista_eventos;

    EstadoTarefa estadoAtual;
    int          tempoRestante;  
    int          quantumRestante; // 0 quando não está em execução

    // histórico por tick: map<tick, estado> alimenta o Gráfico de Gantt.
    // usar map com sobrescrita permite que undo/redo reescreva ticks já calculados
    // sem precisar limpar o histórico manualmente
    std::map<int, EstadoTarefa> historicoNoTempo;

public:
    Tarefa(std::string id, std::string corHex, int ingresso, int duracao,
           int prioridade, std::vector<int> lista_eventos);
    ~Tarefa();

    std::string getID()         const;
    std::string getCorHex()     const;
    int         getIngresso()   const;
    int         getDuracao()    const;
    int         getPrioridade() const;

    EstadoTarefa getEstadoAtual()    const;
    int          getTempoRestante()   const;
    int          getQuantumRestante() const;

    void setEstadoAtual(EstadoTarefa estado);
    void setTempoRestante(int t);
    void setQuantumRestante(int q);
    void decrementarTempoRestante();
    void decrementarQuantumRestante();

    // histórico por tick
    void         registrarEstadoNoTempo(int tick, EstadoTarefa estado);
    EstadoTarefa buscarEstadoNoTempo(int tick) const;
};
