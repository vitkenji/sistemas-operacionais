#pragma once
#include "tarefa/tarefa.hpp"

#include <vector>

class Escalonador
{
public:
    virtual ~Escalonador() = default;
    virtual void atualizarTarefas(std::vector<Tarefa>& tarefas, int tempoAtual) = 0;
};
