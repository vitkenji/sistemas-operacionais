#pragma once
// PriopEscalonador.hpp
// Escalonador por Prioridade Preemptivo (PRIOp) — req. 4 / string "PRIOP".
//
// Critério primário: prioridade estática maior = mais prioritário (req. 4.4).
// Critérios de desempate (req. 4.3), em ordem:
//   1. Tarefa já em execução é preferida (evita troca de contexto desnecessária)
//   2. Ingresso menor (chegou antes)
//   3. Duração menor
//   4. Sorteio (ícone ◆ no Gantt)
//
// Implementação em PriopEscalonador.cpp.

#include "escalonadores/Escalonador.hpp"

class PriopEscalonador : public Escalonador {
public:
    PriopEscalonador()  = default;
    ~PriopEscalonador() override = default;

    // Seleciona as N tarefas mais prioritárias (N = número de CPUs) e as distribui
    // pelas CPUs minimizando trocas de contexto.
    ResultadoEscalonamento escalonar(
        const std::vector<Tarefa>& tarefas,
        const std::vector<CPU>&    cpus,
        int tempoAtual) override;
};
