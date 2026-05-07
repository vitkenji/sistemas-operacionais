#pragma once
#include "escalonadores/Escalonador.hpp"

class PriopEscalonador : public Escalonador {
public:
    PriopEscalonador()  = default;
    ~PriopEscalonador() override = default;

    std::map<int, int> escalonar(
        const std::vector<Tarefa>& tarefas,
        const std::vector<CPU>&    cpus,
        int tempoAtual) override;
};
