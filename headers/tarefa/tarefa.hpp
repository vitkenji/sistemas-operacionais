#pragma once
#include <map>
#include <string>
#include <vector>

enum class EstadoTarefa {
    Nova,
    Pronta,
    Execucao,
    Suspensa,
    Terminada
};

class Tarefa {
private:
    int          ID;
    std::string  corHex;
    int          ingresso;
    int          duracao;
    int          prioridade;
    std::vector<int> lista_eventos;

    // Estado corrente da simulação (modificado pelo motor tick a tick)
    EstadoTarefa estadoAtual;
    int          tempoRestante;
    int          quantumRestante;

    // Histórico por tick (alimenta o Gráfico de Gantt)
    std::map<int, EstadoTarefa> historicoNoTempo;

public:
    Tarefa(int id, std::string corHex, int ingresso, int duracao,
           int prioridade, std::vector<int> lista_eventos);
    ~Tarefa();

    // Atributos fixos
    int         getID()         const;
    std::string getCorHex()     const;
    int         getIngresso()   const;
    int         getDuracao()    const;
    int         getPrioridade() const;

    // Estado corrente (leitura)
    EstadoTarefa getEstadoAtual()    const;
    int          getTempoRestante()   const;
    int          getQuantumRestante() const;

    // Estado corrente (escrita — usada pelo motor e pela edição manual)
    void setEstadoAtual(EstadoTarefa estado);
    void setTempoRestante(int t);
    void setQuantumRestante(int q);
    void decrementarTempoRestante();
    void decrementarQuantumRestante();

    // Histórico por tick
    void         registrarEstadoNoTempo(int tick, EstadoTarefa estado);
    EstadoTarefa buscarEstadoNoTempo(int tick) const;
};
