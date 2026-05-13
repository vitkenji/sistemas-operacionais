#pragma once
#include "config/CarregadorConfig.hpp"

class TelaInicial {
private:
    char            caminhoArquivo[512];
    ConfigSimulacao ultimaConfig;
    bool            tentouCarregar; //flag que controla se mensagem de erro deve aparecer
    bool            simulacaoIniciada;

    void processarImportacao();
    void carregarExemplo(const char* caminho);
    void desenharFormulario();
    void desenharResultado();

public:
    TelaInicial();

    void desenhar();
    bool isSimulacaoIniciada() const;

    // chamado quando usuário volta da tela de simulação
    void resetar();
};
