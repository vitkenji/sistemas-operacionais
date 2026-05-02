#pragma once

class Escalonador
{
public:
    virtual ~Escalonador() = default;
    virtual void atualizarTarefas() = 0;
};
