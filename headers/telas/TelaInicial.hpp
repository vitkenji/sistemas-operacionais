#pragma once
#include <string>

class TelaInicial {
private:
    char caminhoArquivo[256]; 
    
    std::string conteudoLido; // Vai guardar o texto do arquivo depois de lido
    bool arquivoCarregado;    // Controla se mostramos mensagem de sucesso ou não

    void processarImportacao(); // Função que realmente lê o arquivo txt

public:
    // Construtor (configura o estado inicial da tela)
    TelaInicial();

    // --- COMPORTAMENTO EXTERNO ---
    // Essa é a função que o GerenciadorGrafico vai chamar todo frame
    void desenhar(); 
};