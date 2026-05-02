#include "tarefa/tarefa.hpp"
#include <iostream>
#include <utility>

Tarefa::Tarefa(int id, int ingresso, int duracao, int prioridade, std::vector<int> lista_eventos)
    : ID(id),
      ingresso(ingresso),
      duracao(duracao),
      prioridade(prioridade),
      lista_eventos(std::move(lista_eventos))
{
}

Tarefa::~Tarefa() = default;

void Tarefa::registrarEstadoNoTempo(int instanteTempo, EstadoTarefa novoEstado)
{
    historicoNoTempo[instanteTempo] = novoEstado;
    std::cout << "Tempo " << instanteTempo << ": Estado atualizado.\n";
}

EstadoTarefa Tarefa::buscarEstadoNoTempo(int instanteTempo) const
{
    std::map<int, EstadoTarefa>::const_iterator busca = historicoNoTempo.find(instanteTempo);

    if (busca == historicoNoTempo.end())
    {
        return EstadoTarefa::Nova;
    }

    return busca->second;
}
