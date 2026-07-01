#include "tarefa/tarefa.hpp"
#include <utility>

Tarefa::Tarefa(std::string id, std::string corHex, int ingresso, int duracao,
               int prioridade, std::vector<AcaoTarefa> acoes)
    : ID(std::move(id)),
      corHex(std::move(corHex)),
      ingresso(ingresso),
      duracao(duracao),
      prioridade(prioridade),
      prioridadeDinamica(prioridade),
      acoes(std::move(acoes)),
      estadoAtual(EstadoTarefa::Nova),
      motivoSuspensao(MotivoSuspensao::Nenhum),
      tempoRestante(duracao),
      quantumRestante(0),
      proximaAcaoIndex(0)
{
}

Tarefa::~Tarefa() = default;

std::string Tarefa::getID()         const { return ID; }
std::string Tarefa::getCorHex()     const { return corHex; }
int         Tarefa::getIngresso()   const { return ingresso; }
int         Tarefa::getDuracao()    const { return duracao; }
int         Tarefa::getPrioridade() const { return prioridade; }
int         Tarefa::getPrioridadeDinamica() const { return prioridadeDinamica; }
int         Tarefa::getTempoExecutado() const { return duracao - tempoRestante; }
const std::vector<AcaoTarefa>& Tarefa::getAcoes() const { return acoes; }
std::size_t Tarefa::getProximaAcaoIndex() const { return proximaAcaoIndex; }
MotivoSuspensao Tarefa::getMotivoSuspensao() const { return motivoSuspensao; }

EstadoTarefa Tarefa::getEstadoAtual()    const { return estadoAtual; }
int          Tarefa::getTempoRestante()   const { return tempoRestante; }
int          Tarefa::getQuantumRestante() const { return quantumRestante; }

void Tarefa::setEstadoAtual(EstadoTarefa estado)  { estadoAtual = estado; }
void Tarefa::setTempoRestante(int t)              { tempoRestante = t; }
void Tarefa::setQuantumRestante(int q)            { quantumRestante = q; }
void Tarefa::setPrioridadeDinamica(int p)         { prioridadeDinamica = p; }
void Tarefa::setProximaAcaoIndex(std::size_t index) { proximaAcaoIndex = index; }
void Tarefa::setMotivoSuspensao(MotivoSuspensao motivo) { motivoSuspensao = motivo; }
void Tarefa::resetarPrioridadeDinamica()          { prioridadeDinamica = prioridade; }
void Tarefa::incrementarPrioridadeDinamica(int incremento)
{
    prioridadeDinamica += incremento;
}

void Tarefa::avancarAcao()
{
    if (proximaAcaoIndex < acoes.size()) proximaAcaoIndex++;
}

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
