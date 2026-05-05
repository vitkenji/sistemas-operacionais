#pragma once
#include "escalonadores/Escalonador.hpp"

class SRTFEscalonador : public Escalonador
{
public:
    SRTFEscalonador();
    ~SRTFEscalonador();

    void atualizarTarefas(std::vector<Tarefa>& tarefas, int tempoAtual);
};