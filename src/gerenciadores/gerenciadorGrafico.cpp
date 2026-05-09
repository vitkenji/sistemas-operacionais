// gerenciadorGrafico.cpp
// Implementação do gerenciador de janela e renderização.
// Veja gerenciadorGrafico.hpp para a descrição completa das responsabilidades.

#include "gerenciadores/GerenciadorGrafico.hpp"

// Dear ImGui e seus backends
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// stb_image_write: biblioteca header-only para escrita de PNG.
// A macro STB_IMAGE_WRITE_IMPLEMENTATION deve ser definida em exatamente uma
// unidade de compilação para gerar a implementação — aqui é o local correto.
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

// Inicializa GLFW, cria a janela com contexto OpenGL 3.0, e configura o ImGui.
// Retorna false se qualquer etapa falhar (a mensagem de erro vai para stderr).
bool GerenciadorGrafico::inicializar() {
    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar o GLFW!" << std::endl;
        return false;
    }

    // OpenGL 3.0 é suficiente para o ImGui + glReadPixels usados neste projeto
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(largura, altura, titulo.c_str(), NULL, NULL);
    if (!window) {
        std::cerr << "Falha ao criar janela GLFW!" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Habilita V-Sync para evitar tearing e controlar taxa de frames

    // Cria o contexto ImGui com o tema escuro padrão
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Conecta os backends: GLFW processa eventos, OpenGL3 faz o draw
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    return true;
}

bool GerenciadorGrafico::janelaDeveFechar() const {
    return glfwWindowShouldClose(window);
}

// Consulta o sistema operacional por eventos de entrada (teclado, mouse, janela).
void GerenciadorGrafico::processarEventos() {
    glfwPollEvents();
}

// Prepara o ImGui para receber chamadas de widgets neste frame.
// Deve ser chamado antes de qualquer ImGui::Begin/End.
void GerenciadorGrafico::iniciarFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

// Finaliza o frame: renderiza o ImGui na tela, executa captura PNG pendente
// e troca os buffers da janela.
void GerenciadorGrafico::renderizar() {
    ImGui::Render();

    // Atualiza dimensões do viewport (pode ter mudado por resize da janela)
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Escreve os draw calls do ImGui no back-buffer
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Captura ANTES do swap: o back-buffer contém o frame completo com o Gantt renderizado.
    // Feito assim para garantir que glReadPixels lê o frame atual, não o frame anterior.
    if (!caminhoCaptura.empty()) {
        capturarFramebuffer(caminhoCaptura, display_w, display_h);
        caminhoCaptura.clear();
    }

    glfwSwapBuffers(window);
}

// Agenda a captura do próximo frame para o caminho informado.
// A captura efetiva ocorre dentro de renderizar().
void GerenciadorGrafico::pedirCaptura(const std::string& caminho)
{
    caminhoCaptura = caminho;
}

// Lê os pixels do framebuffer atual, inverte verticalmente (OpenGL usa origem em baixo-esquerda
// enquanto PNG usa topo-esquerda) e salva em arquivo PNG via stb_image_write.
void GerenciadorGrafico::capturarFramebuffer(const std::string& caminho, int w, int h)
{
    int stride = w * 4;  // 4 bytes por pixel (RGBA)
    std::vector<unsigned char> pixels((size_t)(stride * h));
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Inversão vertical: linha 0 do OpenGL = parte inferior da tela
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

// Libera todos os recursos ImGui e GLFW.
// O guard 'window != nullptr' impede dupla liberação caso limpar() seja chamado
// explicitamente e depois pelo destrutor.
void GerenciadorGrafico::limpar() {
    if (window != nullptr) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();

        window = nullptr;
    }
}
