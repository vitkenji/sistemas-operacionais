#pragma once

struct CPU {
    int  id;
    int  tarefaAtualID;  // ID da tarefa em execução; -1 = ociosa ou desligada
    bool ligada;          // false = desligada (sem tarefas disponíveis no sistema)
};
