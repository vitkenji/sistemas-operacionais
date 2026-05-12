#pragma once
#include "tarefa/tarefa.hpp"
#include "simulacao/CPU.hpp"
#include <map>
#include <vector>

// Resultado devolvido por Escalonador::escalonar() a cada tick.
struct ResultadoEscalonamento {
    // Mapeamento cpu_id → tarefa_id para este tick.
    // tarefa_id == -1 significa que a CPU fica ociosa.
    std::map<int, int> alocacao;

    // IDs das tarefas cujo empate foi resolvido por sorteio neste tick.
    // O motor repassa essa lista ao snapshot do histórico para que o
    // Gráfico de Gantt exiba o ícone de sorteio (◆) na célula correspondente.
    std::vector<int> sorteadas;
};

// Interface abstrata do escalonador — padrão Strategy.
// Cada algoritmo (PRIOp, SRTF, ...) implementa esta classe e define
// apenas a lógica de escalonamento; o motor (GerenciadorTarefa) não
// precisa saber qual algoritmo está em uso.
//
// Para adicionar um novo algoritmo:
//   1. Criar subclasse em headers/escalonadores/ e src/escalonadores/
//   2. Registrar o nome em GerenciadorTarefa::criarEscalonador()
class Escalonador {
public:
    virtual ~Escalonador() = default;

    // Decide o mapeamento completo CPU → tarefa para o tick atual.
    // Chamado pelo motor após tratar chegadas e finalizações,
    // mas ANTES de decrementar os contadores de tempo e quantum.
    //
    // tarefas:    todas as tarefas do sistema (com estadoAtual, tempoRestante, etc.)
    // cpus:       estado atual das CPUs (incluindo as que já têm tarefa rodando)
    // tempoAtual: tick corrente
    virtual ResultadoEscalonamento escalonar(
        const std::vector<Tarefa>& tarefas,
        const std::vector<CPU>&    cpus,
        int tempoAtual) = 0;
};
