#pragma once
#include "escalonadores/Escalonador.hpp"

class SRTFEscalonador : public Escalonador {
public:
    SRTFEscalonador()  = default;
    ~SRTFEscalonador() override = default;

    bool isPreemptivo() const override { return false; }

    ResultadoEscalonamento escalonar(
        const std::vector<Tarefa>& tarefas,
        const std::vector<CPU>&    cpus,
        int tempoAtual) override;
};
