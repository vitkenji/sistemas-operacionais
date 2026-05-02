#pragma once

class Escalonador
{
public:
    virtual ~Escalonador() = default;
    virtual void atualizarTarefas(std::vector<Tarefa>& tarefas, int tempoAtual) = 0;
};
