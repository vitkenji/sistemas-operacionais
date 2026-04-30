#include "tarefa/tarefa.hpp"

Tarefa::Tarefa(int id, int ingresso, int duracao, int prioridade, std::vector<int> lista_eventos)
{
    this->ID = id;
    this->ingresso = ingresso;
    this->duracao = duracao;
    this->prioridade = prioridade;
    this->lista_eventos = lista_eventos;
}

Tarefa::~Tarefa()
{

}