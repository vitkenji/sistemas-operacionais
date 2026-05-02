#pragma once
#include "tarefa/tarefa.hpp"

#include <map>
#include <vector>

class GerenciadorTarefa
{
private:
    static GerenciadorTarefa* instance;
    std::vector<Tarefa> listaTarefas;
    std::map<EstadoTarefa, int> contagemEstados;

    GerenciadorTarefa();
public:
    ~GerenciadorTarefa();
    static GerenciadorTarefa* getInstance();


};
