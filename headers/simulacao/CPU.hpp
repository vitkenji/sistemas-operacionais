#pragma once
// CPU.hpp
// Representa o estado corrente de um processador (CPU/core) no sistema simulado.
//
// Decisão de design: estrutura simples (POD-like) porque o estado de uma CPU
// é completamente derivado das decisões do escalonador — não há lógica própria.
// O GerenciadorTarefa e o GanttChart lêem e escrevem diretamente os campos.

struct CPU {
    int  id;            // identificador único (0, 1, 2, ...)
    int  tarefaAtualID; // ID da tarefa em execução; -1 = ociosa ou desligada
    bool ligada;        // false = desligada (req. 1.2: sem tarefas prontas disponíveis)
};
