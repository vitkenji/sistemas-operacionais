#pragma once
#include <map>
#include <string>
#include <vector>

// Estados possíveis de uma tarefa ao longo do seu ciclo de vida.
// A transição Nova → Pronta ocorre quando o tick atinge o instante de ingresso.
// Execucao → Pronta pode ocorrer por preempção (quantum ou prioridade).
// Execucao → Terminada ocorre quando tempoRestante chega a zero.
enum class EstadoTarefa {
    Nova,
    Pronta,
    Execucao,
    Suspensa,
    Terminada
};

// TCB (Task Control Block): representa uma tarefa no simulador.
// Armazena tanto os atributos fixos (lidos do arquivo) quanto o estado
// corrente da simulação e o histórico tick-a-tick para o Gráfico de Gantt.
class Tarefa {
private:
    // Atributos fixos — definidos no arquivo de configuração, nunca mudam
    int          ID;
    std::string  corHex;       // cor de exibição no Gantt (RGB hex, ex: "FF4444")
    int          ingresso;     // tick em que a tarefa entra no sistema
    int          duracao;      // tempo total de CPU necessário
    int          prioridade;   // usado pelo PRIOp (maior = mais prioritário)
    std::vector<int> lista_eventos;

    // Estado corrente da simulação — modificado pelo motor tick a tick
    EstadoTarefa estadoAtual;
    int          tempoRestante;   // quanto falta para terminar (começa igual a duracao)
    int          quantumRestante; // fatia restante do quantum atual (0 quando não está em execução)

    // Histórico por tick: map<tick, estado> alimenta o Gráfico de Gantt.
    // Usar map com sobrescrita permite que undo/redo reescreva ticks já calculados
    // sem precisar limpar o histórico manualmente.
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
