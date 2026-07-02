#pragma once
#include "imgui.h"
#include <functional>
#include <string>

struct GLFWwindow;

class GerenciadorGrafico {
private:
    GLFWwindow* window;
    int largura;
    int altura;
    std::string titulo;

public:
    GerenciadorGrafico(int largura, int altura, const std::string& titulo);
    ~GerenciadorGrafico();

    bool inicializar();
    void limpar();

    bool janelaDeveFechar() const;
    void processarEventos();
    void iniciarFrame();
    void renderizar();

    // Renderiza um desenho ImGui fora da tela e salva o resultado em PNG.
    bool exportarPNG(const std::string& caminho,
                     ImVec2 tamanho,
                     const std::function<void(ImDrawList*, ImVec2)>& desenhar);
};
