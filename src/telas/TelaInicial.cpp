#include "telas/TelaInicial.hpp"
#include "imgui.h"
#include <fstream>
#include <sstream>
#include <iostream>

TelaInicial::TelaInicial() {
    // Configurações iniciais quando a tela é criada
    caminhoArquivo[0] = '\0'; // Deixa o campo de texto vazio no começo
    arquivoCarregado = false;
}

void TelaInicial::processarImportacao() {
    // Tenta abrir o arquivo no caminho digitado
    std::ifstream arquivo(caminhoArquivo);
    
    if (arquivo.is_open()) {
        // Se conseguiu abrir, lê tudo e joga na string 'conteudoLido'
        std::stringstream buffer;
        buffer << arquivo.rdbuf();
        conteudoLido = buffer.str();
        
        arquivoCarregado = true;
        arquivo.close();
    } else {
        // Se falhou (arquivo não existe, caminho errado, etc)
        conteudoLido = "Erro: Nao foi possivel abrir o arquivo. Verifique o caminho!";
        arquivoCarregado = false;
    }
}

void TelaInicial::desenhar() {
    // 1. Inicia a janela do ImGui
    ImGui::Begin("Bem-vindo ao Sistema");

    // 2. Textos e campo de digitação
    ImGui::Text("Por favor, importe o seu arquivo de tarefas (.txt)");
    ImGui::Separator(); // Uma linha bonitinha para separar

    ImGui::Text("Caminho do arquivo:");
    // Input de texto do ImGui. Ele salva o que o usuário digitar em 'caminhoArquivo'
    ImGui::InputText("##caminho", caminhoArquivo, IM_ARRAYSIZE(caminhoArquivo));

    // 3. O Botão de Importar
    if (ImGui::Button("Importar Arquivo")) {
        // Se o usuário clicar no botão, chama a função de leitura
        processarImportacao();
    }

    // 4. Feedback visual após clicar no botão
    if (arquivoCarregado) {
        // Texto verde para sucesso
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Arquivo importado com sucesso!");
        
        // (Opcional) Mostra um pedacinho do arquivo lido em uma caixa de texto não editável
        ImGui::Text("Conteudo lido:");
        ImGui::InputTextMultiline("##conteudo", (char*)conteudoLido.c_str(), conteudoLido.size(), 
                                  ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 10), 
                                  ImGuiInputTextFlags_ReadOnly);
                                  
        // Aqui no futuro você poderia colocar um botão: Se(ImGui::Button("Ir para Simulacao")) { ... }
    } else if (!conteudoLido.empty()) {
        // Texto vermelho para erro
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", conteudoLido.c_str());
    }

    // 5. Finaliza a janela
    ImGui::End();
}