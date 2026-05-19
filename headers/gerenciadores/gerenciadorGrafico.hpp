#pragma once
#include <GLFW/glfw3.h>
#include <string>

class GerenciadorGrafico {
private:
    GLFWwindow* window;
    int largura;
    int altura;
    std::string titulo;

    std::string caminhoCaptura;
    float capturaMinX = 0, capturaMinY = 0;
    float capturaMaxX = 0, capturaMaxY = 0;

    // le o framebuffer atual e salva em PNG, recortando para a região indicada.
    // Deve ser chamado após RenderDrawData e antes de SwapBuffers.
    void capturarFramebuffer(const std::string& caminho, int w, int h);

public:
    GerenciadorGrafico(int largura, int altura, const std::string& titulo);
    ~GerenciadorGrafico();

    bool inicializar();
    void limpar();

    bool janelaDeveFechar() const;
    void processarEventos();
    void iniciarFrame();
    void renderizar();

    // agenda captura do próximo frame para o arquivo indicado.
    // minX/minY/maxX/maxY: recorte em pixels lógicos de tela; todos zero = tela inteira.
    void pedirCaptura(const std::string& caminho,
                      float minX, float minY, float maxX, float maxY);
};