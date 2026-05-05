#include "gerenciadores/GerenciadorTarefa.hpp"
#include "escalonadores/PriopEscalonador.hpp"
#include "escalonadores/SRTFEscalonador.hpp"

#include <iostream>

GerenciadorTarefa* GerenciadorTarefa::instance = nullptr;

GerenciadorTarefa* GerenciadorTarefa::getInstance()
{
    return instance;
}

void GerenciadorTarefa::configurar(const ConfigSimulacao& config)
{
    resetar();
    instance = new GerenciadorTarefa(config);
}

void GerenciadorTarefa::resetar()
{
    delete instance;
    instance = nullptr;
}

GerenciadorTarefa::GerenciadorTarefa(const ConfigSimulacao& config)
    : pEscalonador(criarEscalonador(config.algoritmo)),
      qtde_cpus(config.qtde_cpus),
      quantum(config.quantum),
      listaTarefas(config.tarefas)
{
}

GerenciadorTarefa::~GerenciadorTarefa()
{
    delete pEscalonador;
}

void GerenciadorTarefa::avancaTempo(int tempoAtual)
{
    std::cout << "\n=== Avancando relogio para t = " << tempoAtual << " ===\n";
    pEscalonador->atualizarTarefas(listaTarefas, tempoAtual);
    atualizarContagemEstados(tempoAtual);
}

int GerenciadorTarefa::getQuantidadeEstado(EstadoTarefa estado) const
{
    auto busca = contagemEstados.find(estado);
    return (busca != contagemEstados.end()) ? busca->second : 0;
}

int GerenciadorTarefa::getQtdeCpus() const { return qtde_cpus; }
int GerenciadorTarefa::getQuantum()   const { return quantum; }

void GerenciadorTarefa::atualizarContagemEstados(int tempoAtual)
{
    contagemEstados.clear();
    for (auto& tarefa : listaTarefas) {
        EstadoTarefa estado = tarefa.buscarEstadoNoTempo(tempoAtual);
        contagemEstados[estado]++;
    }
}

Escalonador* GerenciadorTarefa::criarEscalonador(const std::string& tipo)
{
    if (tipo == "srtf")
        return new SRTFEscalonador();
    // Padrão: "priop" (e qualquer string desconhecida)
    return new PriopEscalonador();
}
