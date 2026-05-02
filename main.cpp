#include <GLFW/glfw3.h>
#include "gerenciadores/GerenciadorTarefa.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "tarefa/tarefa.hpp"
#include <iostream>
#include <vector>

int main() {
    // 1. Inicializa o GLFW e cria a janela
    if (!glfwInit()) return -1;
    
    // Configura o GLFW para usar OpenGL 3.0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Meu Projeto ImGui", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Habilita V-Sync

    // 2. Inicializa o contexto do ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    // Configura o estilo visual
    ImGui::StyleColorsDark();

    // 3. Inicializa os Backends (GLFW + OpenGL3)
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    GerenciadorTarefa* gerenciadorTarefa = GerenciadorTarefa::getInstance("priop");

    std::vector<int> eventosTarefa1;
    std::vector<int> eventosTarefa2;
    std::vector<int> eventosTarefa3;

    Tarefa tarefa1(1, 0, 5, 1, eventosTarefa1);
    Tarefa tarefa2(2, 1, 3, 2, eventosTarefa2);
    Tarefa tarefa3(3, 2, 4, 3, eventosTarefa3);

    tarefa1.registrarEstadoNoTempo(0, EstadoTarefa::Nova);
    tarefa2.registrarEstadoNoTempo(0, EstadoTarefa::Nova);
    tarefa3.registrarEstadoNoTempo(0, EstadoTarefa::Nova);

    gerenciadorTarefa->adicionarTarefa(tarefa1);
    gerenciadorTarefa->adicionarTarefa(tarefa2);
    gerenciadorTarefa->adicionarTarefa(tarefa3);

    int contadorLoop = 1;
    int tempoAtual = 1;

    // 4. Loop Principal
    while (!glfwWindowShouldClose(window)) {
        if (contadorLoop % 100 == 0)
        {
            gerenciadorTarefa->avancaTempo(tempoAtual);
            tempoAtual++;
        }
        contadorLoop++;

        // Processa os eventos da janela (teclado, mouse, etc)
        glfwPollEvents();

        // Inicia um novo frame do ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- SEU CÓDIGO DE INTERFACE VEM AQUI ---
        ImGui::ShowDemoWindow(); // Mostra a janela de demonstração padrão do ImGui
        
        // Renderiza o ImGui
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f); // Cor de fundo
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Troca os buffers da janela
        glfwSwapBuffers(window);
    }

    // 5. Limpeza final
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
