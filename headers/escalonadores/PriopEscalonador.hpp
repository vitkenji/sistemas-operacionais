#pragma once
#include "escalonadores/Escalonador.hpp"

class PriopEscalonador : public Escalonador
{
public:
    PriopEscalonador();
    ~PriopEscalonador();

    void atualizarTarefas(std::vector<Tarefa>& tarefas, int tempoAtual);
};
