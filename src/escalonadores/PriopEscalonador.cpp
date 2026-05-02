#include "escalonadores/PriopEscalonador.hpp"

#include <iostream>

PriopEscalonador::PriopEscalonador()
{
}

PriopEscalonador::~PriopEscalonador() = default;

void PriopEscalonador::atualizarTarefas(std::vector<Tarefa>& tarefas, int tempoAtual)
{
    std::cout << "[Escalonador Priop] Ajustando tarefas para o tempo " << tempoAtual << "...\n";

    // apenas uma logica ficticia
    for (std::vector<Tarefa>::iterator tarefa = tarefas.begin(); tarefa != tarefas.end(); ++tarefa)
    {
        EstadoTarefa estadoAnterior = tarefa->buscarEstadoNoTempo(tempoAtual - 1);
        tarefa->registrarEstadoNoTempo(tempoAtual, estadoAnterior);
    }
}
