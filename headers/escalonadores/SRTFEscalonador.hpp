#pragma once
// SRTFEscalonador.hpp
// Escalonador SRTF (Shortest Remaining Time First) — req. 4 / string "SRTF".
//
// Critério primário: menor tempo restante = maior prioridade (preemptivo).
// Critérios de desempate (req. 4.3), em ordem:
//   1. Tarefa já em execução é preferida (evita troca de contexto desnecessária)
//   2. Ingresso menor (chegou antes)
//   3. Duração menor
//   4. Sorteio (ícone ◆ no Gantt)
//
// Nota: o quantum ainda é aplicado pelo motor (GerenciadorTarefa::computarProximoTick),
// pois o enunciado define quantum como o "período máximo de tempo que uma tarefa pode
// executar" — válido para todos os algoritmos. O SRTF pode preemptar dentro do quantum
// se uma tarefa com tempo restante menor chegar.
//
// Implementação em SRTFescalonador.cpp.

#include "escalonadores/Escalonador.hpp"

class SRTFEscalonador : public Escalonador {
public:
    SRTFEscalonador()  = default;
    ~SRTFEscalonador() override = default;

    // Seleciona as N tarefas com menor tempo restante (N = número de CPUs) e as distribui
    // pelas CPUs minimizando trocas de contexto.
    ResultadoEscalonamento escalonar(
        const std::vector<Tarefa>& tarefas,
        const std::vector<CPU>&    cpus,
        int tempoAtual) override;
};
