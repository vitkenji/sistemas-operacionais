#include "gerenciadores/GerenciadorGrafico.hpp"
#include "gerenciadores/GerenciadorTarefa.hpp"
#include "telas/TelaInicial.hpp"
#include "tarefa/tarefa.hpp"
#include "imgui.h" // Apenas para o ImGui::ShowDemoWindow();
#include <iostream>
#include <vector>

int main() {
    GerenciadorGrafico gerenciadorGrafico(1280, 720, "Meu Projeto ImGui");

    if (!gerenciadorGrafico.inicializar()) {
        return -1;
    }

    // Instancia a sua nova tela
    TelaInicial telaInicial;

    // ... lógica de tarefas aqui ...

    while (!gerenciadorGrafico.janelaDeveFechar()) {
        
        // ... lógica de avanço de tempo do gerenciador de tarefas ...

        gerenciadorGrafico.processarEventos();
        gerenciadorGrafico.iniciarFrame();

        // --- INTERFACE GRÁFICA ---
        // Em vez do ImGui::ShowDemoWindow();, você chama a sua tela:
        telaInicial.desenhar();

        gerenciadorGrafico.renderizar();
    }

    return 0;
}