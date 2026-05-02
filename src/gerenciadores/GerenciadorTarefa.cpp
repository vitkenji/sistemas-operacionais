#include "gerenciadores/GerenciadorTarefa.hpp"
#include "escalonadores/PriopEscalonador.hpp"

#include <iostream>
#include <string>

GerenciadorTarefa* GerenciadorTarefa::instance = nullptr;

GerenciadorTarefa* GerenciadorTarefa::getInstance(std::string tipoEscalonamento)
{
    if(instance == nullptr)
    {
        instance = new GerenciadorTarefa(tipoEscalonamento);
    }
    return instance;
}

GerenciadorTarefa::GerenciadorTarefa(std::string tipoEscalonamento)
    : pEscalonador(criarEscalonador(tipoEscalonamento))
{
}

GerenciadorTarefa::~GerenciadorTarefa()
{
    delete pEscalonador;
}

void GerenciadorTarefa::adicionarTarefa(Tarefa tarefa)
{
    listaTarefas.push_back(tarefa);
}

void GerenciadorTarefa::avancaTempo(int tempoAtual)
{
    std::cout << "\n=== Avancando relogio para t = " << tempoAtual << " ===\n";
    pEscalonador->atualizarTarefas(listaTarefas, tempoAtual);
    atualizarContagemEstados(tempoAtual);
}

int GerenciadorTarefa::getQuantidadeEstado(EstadoTarefa estado) const
{
    std::map<EstadoTarefa, int>::const_iterator busca = contagemEstados.find(estado);

    if (busca == contagemEstados.end())
    {
        return 0;
    }

    return busca->second;
}

void GerenciadorTarefa::atualizarContagemEstados(int tempoAtual)
{
    contagemEstados.clear();

    for (std::vector<Tarefa>::iterator tarefa = listaTarefas.begin(); tarefa != listaTarefas.end(); ++tarefa)
    {
        EstadoTarefa estado = tarefa->buscarEstadoNoTempo(tempoAtual);
        contagemEstados[estado]++;
    }
}

Escalonador* GerenciadorTarefa::criarEscalonador(std::string tipoEscalonamento)
{
    if (tipoEscalonamento == "priop" || tipoEscalonamento == "PRIOp" || tipoEscalonamento == "Priop")
    {
        return new PriopEscalonador();
    }

    return new PriopEscalonador();
}
