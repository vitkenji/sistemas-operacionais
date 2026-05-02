#include "escalonadores/PriopEscalonador.hpp"

#include <cstdlib>
#include <ctime>
#include <iostream>

PriopEscalonador::PriopEscalonador()
{
    std::srand(std::time(nullptr));
}

PriopEscalonador::~PriopEscalonador() = default;

void PriopEscalonador::atualizarTarefas(std::vector<Tarefa>& tarefas, int tempoAtual)
{
    std::cout << "[Escalonador Priop] Ajustando tarefas para o tempo " << tempoAtual << "...\n";

    // Logica ficticia de teste: percorre todas as tarefas e avanca o estado de cada uma.
    for (std::vector<Tarefa>::iterator tarefa = tarefas.begin(); tarefa != tarefas.end(); ++tarefa)
    {
        EstadoTarefa estadoAnterior = tarefa->buscarEstadoNoTempo(tempoAtual - 1);
        EstadoTarefa novoEstado = estadoAnterior;
        int idTarefa = tarefa->getID();

        if (estadoAnterior == EstadoTarefa::Nova)
        {
            novoEstado = EstadoTarefa::Pronta;
            std::cout << "Tarefa " << idTarefa << ": Nova -> Pronta\n";
        }
        else if (estadoAnterior == EstadoTarefa::Pronta)
        {
            novoEstado = EstadoTarefa::Execucao;
            std::cout << "Tarefa " << idTarefa << ": Pronta -> Execucao\n";
        }
        else if (estadoAnterior == EstadoTarefa::Execucao)
        {
            int chance = std::rand() % 100;

            if (chance < 80)
            {
                novoEstado = EstadoTarefa::Suspensa;
                std::cout << "Tarefa " << idTarefa << ": Execucao -> Suspensa\n";
            }
            else
            {
                novoEstado = EstadoTarefa::Terminada;
                std::cout << "Tarefa " << idTarefa << ": Execucao -> Terminada\n";
            }
        }
        else if (estadoAnterior == EstadoTarefa::Suspensa)
        {
            int quantidadeSuspensa = 0;
            int tempoBusca = tempoAtual - 1;

            while (tempoBusca >= 0 && tarefa->buscarEstadoNoTempo(tempoBusca) == EstadoTarefa::Suspensa)
            {
                quantidadeSuspensa++;
                tempoBusca--;
            }

            if (quantidadeSuspensa >= 3)
            {
                novoEstado = EstadoTarefa::Pronta;
                std::cout << "Tarefa " << idTarefa << ": Suspensa -> Pronta\n";
            }
            else
            {
                novoEstado = EstadoTarefa::Suspensa;
                std::cout << "Tarefa " << idTarefa << ": Suspensa -> Suspensa ("
                          << quantidadeSuspensa << "/3)\n";
            }
        }
        else if (estadoAnterior == EstadoTarefa::Terminada)
        {
            novoEstado = EstadoTarefa::Terminada;
            std::cout << "Tarefa " << idTarefa << ": Terminada -> Terminada\n";
        }

        tarefa->registrarEstadoNoTempo(tempoAtual, novoEstado);
    }
}
