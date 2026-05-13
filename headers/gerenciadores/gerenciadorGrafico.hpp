#pragma once
#include <GLFW/glfw3.h>
#include <string>

class GerenciadorGrafico {
private:
    GLFWwindow* window;
    int largura;
    int altura;
    std::string titulo;

    std::string caminhoCaptura;  // não vazio = capturar antes do próximo swap

    // le o framebuffer atual e salva em PNG. Deve ser chamado após RenderDrawData
    // e antes de SwapBuffers, para garantir que o frame está completo no back-buffer.
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

    // agenda captura do próximo frame renderizado para o arquivo indicado.
    void pedirCaptura(const std::string& caminho);
};