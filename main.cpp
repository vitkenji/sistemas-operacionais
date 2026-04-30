#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

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

    // 4. Loop Principal
    while (!glfwWindowShouldClose(window)) {
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