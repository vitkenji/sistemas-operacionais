#pragma once
#include "tarefa/tarefa.hpp"
#include "simulacao/CPU.hpp"
#include <map>
#include <vector>

// resultado devolvido por Escalonador::escalonar() a cada tick
struct ResultadoEscalonamento {
    // mapeamento cpu_id -> tarefa_id para este tick.
    // tarefa_id == "" -> CPU fica ociosa.
    std::map<int, std::string> alocacao;

    // IDs das tarefas cujo empate foi resolvido por sorteio neste tick.
    std::vector<std::string> sorteadas;
};

class Escalonador {
public:
    virtual ~Escalonador() = default;

    // decide o mapeamento completo CPU -> tarefa para o tick atual.
    // chamado pelo motor após tratar chegadas e finalizações,
    // mas antes de decrementar os contadores de tempo e quantum.
    
    // tarefas:    todas as tarefas do sistema com estadoAtual, tempoRestante etc 
    // cpus:       estado atual das CPUs
    // tempoAtual: tick corrente
    virtual ResultadoEscalonamento escalonar(
        const std::vector<Tarefa>& tarefas,
        const std::vector<CPU>&    cpus,
        int tempoAtual) = 0;
};
