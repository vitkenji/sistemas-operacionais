#pragma once
// gerenciadorGrafico.hpp
// Wrapper do ciclo de vida GLFW + Dear ImGui + OpenGL 3 (req. 5).
//
// Responsabilidades:
//   - Criar e destruir a janela GLFW e o contexto OpenGL
//   - Inicializar/finalizar o contexto ImGui e os backends GLFW/OpenGL3
//   - Gerenciar o loop de frames: processarEventos → iniciarFrame → [UI] → renderizar
//   - Capturar o framebuffer completo e salvar em PNG via stb_image_write (req. 2.4)
//
// Decisão de design: a captura PNG é feita APÓS ImGui_ImplOpenGL3_RenderDrawData
// e ANTES de glfwSwapBuffers, usando glReadPixels no back-buffer. Isso garante
// que o frame completo (incluindo o Gantt renderizado naquele frame) esteja disponível.

#include <GLFW/glfw3.h>
#include <string>

class GerenciadorGrafico {
private:
    GLFWwindow* window;
    int         largura;
    int         altura;
    std::string titulo;

    // Caminho para o PNG a ser capturado no próximo ciclo de renderização.
    // Vazio = nenhuma captura pendente.
    std::string caminhoCaptura;

    // Lê o framebuffer atual via glReadPixels e salva em PNG usando stb_image_write.
    // Inverte as linhas verticalmente porque OpenGL usa origem em baixo-esquerda.
    void capturarFramebuffer(const std::string& caminho, int w, int h);

public:
    GerenciadorGrafico(int largura, int altura, const std::string& titulo);
    ~GerenciadorGrafico();

    // Inicializa GLFW, cria a janela e configura ImGui. Retorna false em falha.
    bool inicializar();
    // Libera todos os recursos GLFW/ImGui. Chamado pelo destrutor.
    void limpar();

    // Retorna true quando o usuário fechou a janela (condição de saída do loop).
    bool janelaDeveFechar() const;
    // Processa eventos de janela/teclado/mouse (glfwPollEvents).
    void processarEventos();
    // Inicia o frame ImGui (deve ser chamado antes de qualquer chamada ImGui::*).
    void iniciarFrame();
    // Renderiza o frame ImGui, executa captura PNG pendente e troca os buffers.
    void renderizar();

    // Agenda a captura do próximo frame para o caminho indicado.
    // A captura ocorre dentro de renderizar(), após RenderDrawData e antes de SwapBuffers.
    void pedirCaptura(const std::string& caminho);
};
