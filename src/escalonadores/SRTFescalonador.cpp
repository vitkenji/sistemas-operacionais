#include "escalonadores/SRTFEscalonador.hpp"

#include <cstdlib>
#include <ctime>
#include <iostream>

SRTFEscalonador::SRTFEscalonador()
{
     std::srand(std::time(nullptr));

}

SRTFEscalonador::~SRTFEscalonador() = default;

void SRTFEscalonador::atualizarTarefas(std::vector<Tarefa>& tarefas, int tempoAtual)
{

}