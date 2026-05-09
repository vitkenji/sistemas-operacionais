// tarefa.cpp
// Implementação do TCB (Task Control Block).
// Veja tarefa.hpp para a descrição completa dos campos e decisões de design.

#include "tarefa/tarefa.hpp"
#include <utility>

// Constrói a tarefa com os parâmetros lidos do arquivo de configuração.
// tempoRestante começa igual à duração total (ainda não executou nada).
// quantumRestante começa em 0; será inicializado pelo motor ao primeiro escalonamento.
Tarefa::Tarefa(int id, std::string corHex, int ingresso, int duracao,
               int prioridade, std::vector<int> lista_eventos)
    : ID(id),
      corHex(std::move(corHex)),
      ingresso(ingresso),
      duracao(duracao),
      prioridade(prioridade),
      lista_eventos(std::move(lista_eventos)),
      estadoAtual(EstadoTarefa::Nova),
      tempoRestante(duracao),
      quantumRestante(0)
{
}

Tarefa::~Tarefa() = default;

// ── Getters de parâmetros fixos ───────────────────────────────────────────────
int         Tarefa::getID()         const { return ID; }
std::string Tarefa::getCorHex()     const { return corHex; }
int         Tarefa::getIngresso()   const { return ingresso; }
int         Tarefa::getDuracao()    const { return duracao; }
int         Tarefa::getPrioridade() const { return prioridade; }

// ── Getters de estado corrente ────────────────────────────────────────────────
EstadoTarefa Tarefa::getEstadoAtual()    const { return estadoAtual; }
int          Tarefa::getTempoRestante()   const { return tempoRestante; }
int          Tarefa::getQuantumRestante() const { return quantumRestante; }

// ── Setters de estado corrente ────────────────────────────────────────────────
void Tarefa::setEstadoAtual(EstadoTarefa estado)  { estadoAtual = estado; }
void Tarefa::setTempoRestante(int t)              { tempoRestante = t; }
void Tarefa::setQuantumRestante(int q)            { quantumRestante = q; }

// Decrementa sem ultrapassar zero para evitar underflow não intencional.
void Tarefa::decrementarTempoRestante()
{
    if (tempoRestante > 0) tempoRestante--;
}

void Tarefa::decrementarQuantumRestante()
{
    if (quantumRestante > 0) quantumRestante--;
}

// ── Histórico por tick ────────────────────────────────────────────────────────

// Registra o estado da tarefa no tick informado.
// Chamado pelo motor ao final de cada tick para alimentar o Gráfico de Gantt.
void Tarefa::registrarEstadoNoTempo(int tick, EstadoTarefa estado)
{
    historicoNoTempo[tick] = estado;
}

// Retorna o estado registrado para o tick informado.
// Se o tick não foi registrado (ex.: antes do ingresso), retorna Nova como sentinela.
EstadoTarefa Tarefa::buscarEstadoNoTempo(int tick) const
{
    auto it = historicoNoTempo.find(tick);
    return (it != historicoNoTempo.end()) ? it->second : EstadoTarefa::Nova;
}
