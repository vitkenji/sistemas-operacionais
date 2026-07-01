#pragma once
#include "escalonadores/Escalonador.hpp"

class PriopDEscalonador : public Escalonador {
public:
    PriopDEscalonador()  = default;
    ~PriopDEscalonador() override = default;

    bool isPreemptivo() const override { return true; }

    ResultadoEscalonamento escalonar(
        std::vector<Tarefa>&       tarefas,
        const std::vector<CPU>&    cpus,
        int tempoAtual) override;
};
