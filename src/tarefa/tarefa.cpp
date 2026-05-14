#include "tarefa/tarefa.hpp"
#include <utility>

Tarefa::Tarefa(std::string id, std::string corHex, int ingresso, int duracao,
               int prioridade, std::vector<int> lista_eventos)
    : ID(std::move(id)),
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

std::string Tarefa::getID()         const { return ID; }
std::string Tarefa::getCorHex()     const { return corHex; }
int         Tarefa::getIngresso()   const { return ingresso; }
int         Tarefa::getDuracao()    const { return duracao; }
int         Tarefa::getPrioridade() const { return prioridade; }

EstadoTarefa Tarefa::getEstadoAtual()    const { return estadoAtual; }
int          Tarefa::getTempoRestante()   const { return tempoRestante; }
int          Tarefa::getQuantumRestante() const { return quantumRestante; }

void Tarefa::setEstadoAtual(EstadoTarefa estado)  { estadoAtual = estado; }
void Tarefa::setTempoRestante(int t)              { tempoRestante = t; }
void Tarefa::setQuantumRestante(int q)            { quantumRestante = q; }

void Tarefa::decrementarTempoRestante()
{
    if (tempoRestante > 0) tempoRestante--;
}

void Tarefa::decrementarQuantumRestante()
{
    if (quantumRestante > 0) quantumRestante--;
}

void Tarefa::registrarEstadoNoTempo(int tick, EstadoTarefa estado)
{
    historicoNoTempo[tick] = estado;
}

EstadoTarefa Tarefa::buscarEstadoNoTempo(int tick) const
{
    auto it = historicoNoTempo.find(tick);
    return (it != historicoNoTempo.end()) ? it->second : EstadoTarefa::Nova;
}
