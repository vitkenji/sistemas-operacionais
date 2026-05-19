#include "gerenciadores/gerenciadorGrafico.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>

GerenciadorGrafico::GerenciadorGrafico(int largura, int altura, const std::string& titulo)
    : largura(largura), altura(altura), titulo(titulo), window(nullptr) {}

GerenciadorGrafico::~GerenciadorGrafico() {
    limpar();
}

bool GerenciadorGrafico::inicializar() {
    // inicializa glfw
    if (!glfwInit())
    {
        std::cerr << "erro ao inicializar GLFW" << std::endl;
        return false;
    }

    // define versao do opengl
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // cria a instancia da janela
    window = glfwCreateWindow(largura, altura, titulo.c_str(), NULL, NULL);
    
    // verifica se a janela foi criada
    if (!window)
    {
        std::cerr << "erro ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return false;
    }

    // define a janela como contexto atual
    glfwMakeContextCurrent(window);
    // habilita v-sync
    glfwSwapInterval(1); 

    // inicializa contexto do imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // define tema escuro
    ImGui::StyleColorsDark();

    // vincula imgui ao glfw e opengl
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    return true;
}

bool GerenciadorGrafico::janelaDeveFechar() const {
    // verifica se a janela deve ser fechada
    return glfwWindowShouldClose(window);
}

void GerenciadorGrafico::processarEventos() {
    // processa inputs e eventos do sistema
    glfwPollEvents();
}

void GerenciadorGrafico::iniciarFrame() {
    // prepara o inicio de um novo frame no imgui
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

void GerenciadorGrafico::pedirCaptura(const std::string& caminho,
                                       float minX, float minY, float maxX, float maxY)
{
    caminhoCaptura = caminho;
    capturaMinX = minX; capturaMinY = minY;
    capturaMaxX = maxX; capturaMaxY = maxY;
}

void GerenciadorGrafico::capturarFramebuffer(const std::string& caminho, int w, int h)
{
    // escala lógico → físico (geralmente 1.0 no Windows; >1 em telas HiDPI)
    ImGuiIO& io = ImGui::GetIO();
    float sx = io.DisplayFramebufferScale.x;
    float sy = io.DisplayFramebufferScale.y;

    // determina a região de recorte em pixels físicos
    int x0, y0, cw, ch;
    if (capturaMaxX > capturaMinX && capturaMaxY > capturaMinY) {
        const float pad = 6.f;
        x0 = std::max(0,  (int)((capturaMinX - pad) * sx));
        y0 = std::max(0,  (int)((capturaMinY - pad) * sy));
        int x1 = std::min(w, (int)((capturaMaxX + pad) * sx));
        int y1 = std::min(h, (int)((capturaMaxY + pad) * sy));
        cw = x1 - x0;
        ch = y1 - y0;
    } else {
        x0 = 0; y0 = 0; cw = w; ch = h;
    }

    if (cw <= 0 || ch <= 0) { x0 = 0; y0 = 0; cw = w; ch = h; }

    // lê o framebuffer inteiro (OpenGL: y=0 é a base da tela)
    int fullStride = w * 4;
    std::vector<unsigned char> pixels((size_t)(fullStride * h));
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // recorta e inverte Y (OpenGL base-esquerda → PNG topo-esquerda)
    int outStride = cw * 4;
    std::vector<unsigned char> out((size_t)(outStride * ch));
    for (int row = 0; row < ch; ++row) {
        int glRow = h - 1 - (y0 + row);
        const unsigned char* src = pixels.data() + glRow * fullStride + x0 * 4;
        std::memcpy(out.data() + row * outStride, src, (size_t)outStride);
    }

    if (!stbi_write_png(caminho.c_str(), cw, ch, 4, out.data(), outStride))
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
