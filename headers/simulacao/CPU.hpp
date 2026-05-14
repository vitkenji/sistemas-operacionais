#pragma once
#include <string>

struct CPU {
    int         id;
    std::string tarefaAtualID;  // id da tarefa executando no cpu; "" = ociosa ou desligada
    bool        ligada;
};
