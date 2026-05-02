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

void Tarefa::mostrarEstadoNoTempo(int instanteTempo) const
{
    auto busca = historicoNoTempo.find(instanteTempo);

    if (busca != historicoNoTempo.end())
    {
        std::cout << "Na tarefa " << ID << ", no tempo " << instanteTempo << ", o estado era: ";
        imprimirEstado(busca->second);
    }
    else
    {
        std::cout << "Nenhum estado mapeado para o tempo " << instanteTempo << ".\n";
    }
}

void Tarefa::mostrarLinhaDoTempo() const
{
    std::cout << "\n--- Linha do Tempo da Tarefa " << ID << " ---\n";
    for (const auto& par : historicoNoTempo)
    {
        std::cout << "Tempo " << par.first << " -> ";
        imprimirEstado(par.second);
    }
    std::cout << "-----------------------------------\n";
}
