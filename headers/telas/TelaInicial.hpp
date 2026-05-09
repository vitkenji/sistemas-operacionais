#pragma once
// TelaInicial.hpp
// Primeira tela exibida ao usuário: importação do arquivo de configuração
// e exibição/edição dos parâmetros antes de iniciar a simulação.
// Responsabilidade: validar a configuração e repassar ao GerenciadorTarefa.

#include "config/CarregadorConfig.hpp"

class TelaInicial {
private:
    char            caminhoArquivo[512];
    ConfigSimulacao ultimaConfig;
    bool            tentouCarregar;
    bool            simulacaoIniciada;

    // Parâmetros editáveis pelo usuário (req. 3.2):
    // preenchidos com defaults na construção e sobrescritos pelo arquivo importado.
    int  algoritmoIdx;      // 0 = PRIOP, 1 = SRTF
    int  quantumEditado;
    int  qtdeCpusEditado;

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
