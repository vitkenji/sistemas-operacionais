#pragma once
#include "escalonadores/Escalonador.hpp"

class PriopEscalonador : public Escalonador {
public:
    PriopEscalonador()  = default;
    ~PriopEscalonador() override = default;

    bool isPreemptivo() const override { return false; }

    ResultadoEscalonamento escalonar(
        const std::vector<Tarefa>& tarefas,
        const std::vector<CPU>&    cpus,
        int tempoAtual) override;
};
