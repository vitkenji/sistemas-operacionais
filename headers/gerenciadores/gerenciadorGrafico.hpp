#pragma once

#include <GLFW/glfw3.h>
#include <string>

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
};