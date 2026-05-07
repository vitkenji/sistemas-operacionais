#pragma once
#include "config/CarregadorConfig.hpp"

class TelaInicial {
private:
    char            caminhoArquivo[512];
    ConfigSimulacao ultimaConfig;
    bool            tentouCarregar;
    bool            simulacaoIniciada;

    void processarImportacao();
    void desenharFormulario();
    void desenharResultado();

public:
    TelaInicial();

    void desenhar();
    bool isSimulacaoIniciada() const;

    // Chamado quando o usuário volta da tela de simulação
    void resetar();
};
