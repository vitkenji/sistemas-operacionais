#include "gerenciadores/GerenciadorGrafico.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <cstring>
#include <iostream>
#include <vector>

GerenciadorGrafico::GerenciadorGrafico(int largura, int altura, const std::string& titulo)
    : largura(largura), altura(altura), titulo(titulo), window(nullptr) {}

GerenciadorGrafico::~GerenciadorGrafico() {
    limpar();
}

bool GerenciadorGrafico::inicializar() {
    // inicializa GLFW
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

    // inicializa contexto do ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // inicializa GLFW + OpenGL
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
    // renderiza interface
    ImGui::Render();

    // atualiza o viewport e limpa a tela de fundo
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    // desenha os dados na tela
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // captura do framebuffer ANTES do swap: o back-buffer contém o frame completo.
    if (!caminhoCaptura.empty()) {
        capturarFramebuffer(caminhoCaptura, display_w, display_h);
        caminhoCaptura.clear();
    }

    // troca os buffers da janela
    glfwSwapBuffers(window);
}

void GerenciadorGrafico::pedirCaptura(const std::string& caminho)
{
    caminhoCaptura = caminho;
}

void GerenciadorGrafico::capturarFramebuffer(const std::string& caminho, int w, int h)
{
    int stride = w * 4;
    std::vector<unsigned char> pixels((size_t)(stride * h));
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // OpenGL: origem em baixo-esquerda -> inverte linhas para o formato PNG (topo-esquerda)
    std::vector<unsigned char> flipped((size_t)(stride * h));
    for (int y = 0; y < h; ++y)
        std::memcpy(flipped.data() + y * stride,
                    pixels.data() + (h - 1 - y) * stride,
                    (size_t)stride);

    if (!stbi_write_png(caminho.c_str(), w, h, 4, flipped.data(), stride))
        std::cerr << "[GerenciadorGrafico] Falha ao salvar PNG: " << caminho << '\n';
    else
        std::cout << "[GerenciadorGrafico] Gantt exportado: " << caminho << '\n';
}

void GerenciadorGrafico::limpar() {
    if (window != nullptr) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
        
        window = nullptr; // garante que nao limpa duas vezes
    }
}