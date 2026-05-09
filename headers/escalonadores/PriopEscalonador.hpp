#pragma once
#include "escalonadores/Escalonador.hpp"

class PriopEscalonador : public Escalonador {
public:
    PriopEscalonador()  = default;
    ~PriopEscalonador() override = default;

    ResultadoEscalonamento escalonar(
        const std::vector<Tarefa>& tarefas,
        const std::vector<CPU>&    cpus,
        int tempoAtual) override;
};
