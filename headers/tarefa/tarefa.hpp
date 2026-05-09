#pragma once
// tarefa.hpp
// Define a estrutura TCB (Task Control Block) — o bloco de controle de tarefa.
// Cada instância de Tarefa armazena TODAS as informações de uma tarefa ao longo
// de todo o ciclo de vida da simulação: parâmetros fixos de entrada, estado corrente
// manipulado pelo motor tick-a-tick, e histórico por tick para o Gráfico de Gantt.
//
// Decisão de design: manter parâmetros fixos e estado corrente na mesma classe
// simplifica a passagem de contexto para o escalonador (ele recebe apenas um vetor
// de Tarefa e acessa o que precisar via getters).

#include <map>
#include <string>
#include <vector>

// Todos os estados possíveis de uma tarefa durante a simulação.
// O motor avança o estado conforme as regras do SO simulado:
//   Nova → Pronta (ao atingir o instante de ingresso)
//   Pronta → Execucao (quando o escalonador atribui uma CPU)
//   Execucao → Pronta (preempção por quantum ou por tarefa de maior prioridade/menor tempo)
//   Execucao → Suspensa (evento de I/O ou mutex — Projeto B)
//   Execucao → Terminada (tempoRestante chega a zero)
enum class EstadoTarefa {
    Nova,
    Pronta,
    Execucao,
    Suspensa,
    Terminada
};

class Tarefa {
private:
    // ── Parâmetros fixos (lidos do arquivo de configuração, nunca alterados) ──
    int          ID;
    std::string  corHex;       // cor RGB em hexadecimal ("F0E0D0") para o Gráfico de Gantt
    int          ingresso;     // tick em que a tarefa chega ao sistema
    int          duracao;      // tempo total de CPU necessário
    int          prioridade;   // prioridade estática (usada pelo PRIOp)
    std::vector<int> lista_eventos;  // ticks de ocorrência de eventos (Projeto B)

    // ── Estado corrente (modificado pelo motor a cada tick) ───────────────────
    EstadoTarefa estadoAtual;
    int          tempoRestante;    // CPU ainda necessária; decrementado a cada tick em Execucao
    int          quantumRestante;  // ticks restantes no quantum atual; reiniciado ao assumir CPU

    // ── Histórico por tick (alimenta o Gráfico de Gantt e o undo/redo) ───────
    // Mapeia tick → estado naquele tick. Permite reconstruir o Gantt sem
    // depender de snapshots globais — cada tarefa carrega seu próprio histórico.
    std::map<int, EstadoTarefa> historicoNoTempo;

public:
    Tarefa(int id, std::string corHex, int ingresso, int duracao,
           int prioridade, std::vector<int> lista_eventos);
    ~Tarefa();

    // ── Leitura de parâmetros fixos ───────────────────────────────────────────
    int         getID()         const;
    std::string getCorHex()     const;
    int         getIngresso()   const;
    int         getDuracao()    const;
    int         getPrioridade() const;

    // ── Leitura do estado corrente ────────────────────────────────────────────
    EstadoTarefa getEstadoAtual()    const;
    int          getTempoRestante()   const;
    int          getQuantumRestante() const;

    // ── Escrita do estado corrente ────────────────────────────────────────────
    // Usado pelo motor (computarProximoTick) e pela edição manual do usuário.
    void setEstadoAtual(EstadoTarefa estado);
    void setTempoRestante(int t);
    void setQuantumRestante(int q);
    void decrementarTempoRestante();
    void decrementarQuantumRestante();

    // ── Histórico por tick ────────────────────────────────────────────────────
    // registrarEstadoNoTempo é chamado pelo motor ao final de cada tick.
    // buscarEstadoNoTempo é usado pelo GanttChart para desenhar o histórico.
    void         registrarEstadoNoTempo(int tick, EstadoTarefa estado);
    EstadoTarefa buscarEstadoNoTempo(int tick) const;
};
