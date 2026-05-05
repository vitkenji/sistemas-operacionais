#include "tarefa/tarefa.hpp"
#include <iostream>
#include <utility>

Tarefa::Tarefa(int id, std::string corHex, int ingresso, int duracao,
               int prioridade, std::vector<int> lista_eventos)
    : ID(id),
      corHex(std::move(corHex)),
      ingresso(ingresso),
      duracao(duracao),
      prioridade(prioridade),
      lista_eventos(std::move(lista_eventos))
{
}

Tarefa::~Tarefa() = default;

int         Tarefa::getID()         const { return ID; }
std::string Tarefa::getCorHex()     const { return corHex; }
int         Tarefa::getIngresso()   const { return ingresso; }
int         Tarefa::getDuracao()    const { return duracao; }
int         Tarefa::getPrioridade() const { return prioridade; }

void Tarefa::registrarEstadoNoTempo(int instanteTempo, EstadoTarefa novoEstado)
{
    historicoNoTempo[instanteTempo] = novoEstado;
}

EstadoTarefa Tarefa::buscarEstadoNoTempo(int instanteTempo) const
{
    auto busca = historicoNoTempo.find(instanteTempo);
    if (busca == historicoNoTempo.end())
        return EstadoTarefa::Nova;
    return busca->second;
}
