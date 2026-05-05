#pragma once
#include "config/CarregadorConfig.hpp"

class TelaInicial {
private:
    char            caminhoArquivo[512];
    ConfigSimulacao ultimaConfig;
    bool            tentouCarregar;

    void processarImportacao();
    void desenharFormulario();
    void desenharResultado();

public:
    TelaInicial();
    void desenhar();
};
