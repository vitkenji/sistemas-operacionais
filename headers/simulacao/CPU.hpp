#pragma once

struct CPU {
    int  id; //id do cpu
    int  tarefaAtualID;  // id da tarefa executando no cpu ; -1 = ociosa ou desligada
    bool ligada;  
};
