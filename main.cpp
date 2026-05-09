// main.cpp
// Ponto de entrada do Simulador de SO Multitarefa Preemptivo.
//
// Fluxo do programa:
//   1. Cria a janela GLFW/ImGui via GerenciadorGrafico.
//   2. Exibe TelaInicial até que o usuário importe um arquivo e clique "Iniciar".
//      TelaInicial configura o GerenciadorTarefa (Singleton) antes de sair.
//   3. Exibe TelaSimulacao, que permite navegar passo-a-passo, executar completamente,
//      editar estados e exportar o Gantt como PNG.
//   4. Se o usuário clicar "Voltar", TelaInicial é restaurada (TelaSimulacao::resetar).
//
// A exportação PNG é tratada de forma especial: TelaSimulacao::consumirPedidoExportacao()
// retorna o caminho solicitado, e GerenciadorGrafico::pedirCaptura() agenda a captura para
// ocorrer dentro de renderizar() — após RenderDrawData e antes de SwapBuffers —
// garantindo que o framebuffer contém o frame completo com o Gantt visível.

#include "gerenciadores/GerenciadorGrafico.hpp"
#include "gerenciadores/GerenciadorTarefa.hpp"
#include "telas/TelaInicial.hpp"
#include "telas/TelaSimulacao.hpp"

int main()
{
    GerenciadorGrafico gerenciadorGrafico(1280, 720, "Simulador SO Multitarefa");

    if (!gerenciadorGrafico.inicializar())
        return -1;

    TelaInicial   telaInicial;
    TelaSimulacao telaSimulacao;

    while (!gerenciadorGrafico.janelaDeveFechar()) {
        gerenciadorGrafico.processarEventos();
        gerenciadorGrafico.iniciarFrame();

        if (!telaInicial.isSimulacaoIniciada()) {
            telaInicial.desenhar();
        } else {
            GerenciadorTarefa* g = GerenciadorTarefa::getInstance();
            bool voltar = telaSimulacao.desenhar(g);

            // Verifica pedido de exportação PNG ANTES de renderizar,
            // para que a captura ocorra no mesmo frame (req. 2.4)
            std::string pngPath = telaSimulacao.consumirPedidoExportacao();
            if (!pngPath.empty())
                gerenciadorGrafico.pedirCaptura(pngPath);

            if (voltar)
                telaInicial.resetar();
        }

        gerenciadorGrafico.renderizar();
    }

    // Libera o Singleton antes de sair para evitar vazamentos de memória
    GerenciadorTarefa::resetar();
    return 0;
}
