#pragma once
#include "escalonadores/Escalonador.hpp"
#include "tarefa/tarefa.hpp"

#include <map>
#include <string>
#include <vector>

class GerenciadorTarefa
{
private:
    static GerenciadorTarefa* instance;
    std::vector<Tarefa> listaTarefas;
    std::map<EstadoTarefa, int> contagemEstados;
    Escalonador* pEscalonador;

    GerenciadorTarefa(std::string tipoEscalonamento);
    Escalonador* criarEscalonador(std::string tipoEscalonamento);
    void atualizarContagemEstados(int tempoAtual);
public:
    ~GerenciadorTarefa();
    static GerenciadorTarefa* getInstance(std::string tipoEscalonamento = "priop");
    void adicionarTarefa(Tarefa tarefa);
    void avancaTempo(int tempoAtual);
    int getQuantidadeEstado(EstadoTarefa estado) const;


};
