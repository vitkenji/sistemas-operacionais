#pragma once
#include <iostream>
#include <vector>

class Tarefa
{
private:
    int ID;
    int ingresso;
    int duracao;
    int prioridade;
    std::vector<int> lista_eventos;

public:
    Tarefa(int id, int ingresso, int duracao, int prioridade, std::vector<int> lista_eventos);
    ~Tarefa();
};