#include "gerenciadores/GerenciadorGrafico.hpp"

// Dependências do ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>

GerenciadorGrafico::GerenciadorGrafico(int largura, int altura, const std::string& titulo)
    : largura(largura), altura(altura), titulo(titulo), window(nullptr) {}

GerenciadorGrafico::~GerenciadorGrafico() {
    limpar();
}

bool GerenciadorGrafico::inicializar() {
    // 1. Inicializa o GLFW
    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar o GLFW!" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(largura, altura, titulo.c_str(), NULL, NULL);
    if (!window) {
        std::cerr << "Falha ao criar janela GLFW!" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Habilita V-Sync

    // 2. Inicializa o contexto do ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // 3. Inicializa os Backends (GLFW + OpenGL3)
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    return true;
}

bool GerenciadorGrafico::janelaDeveFechar() const {
    return glfwWindowShouldClose(window);
}

void GerenciadorGrafico::processarEventos() {
    glfwPollEvents();
}

void GerenciadorGrafico::iniciarFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GerenciadorGrafico::renderizar() {
    // Renderiza a interface do ImGui
    ImGui::Render();
    
    // Atualiza o viewport e limpa a tela de fundo
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Desenha os dados do ImGui na tela
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Troca os buffers da janela
    glfwSwapBuffers(window);
}

void GerenciadorGrafico::limpar() {
    if (window != nullptr) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
        
        window = nullptr; // Garante que não vamos tentar limpar duas vezes
    }
}