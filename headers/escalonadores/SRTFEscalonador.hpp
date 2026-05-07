#pragma once
#include "escalonadores/Escalonador.hpp"

class SRTFEscalonador : public Escalonador {
public:
    SRTFEscalonador()  = default;
    ~SRTFEscalonador() override = default;

    std::map<int, int> escalonar(
        const std::vector<Tarefa>& tarefas,
        const std::vector<CPU>&    cpus,
        int tempoAtual) override;
};
