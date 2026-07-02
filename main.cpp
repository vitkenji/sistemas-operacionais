#include "gerenciadores/gerenciadorGrafico.hpp"
#include "gerenciadores/GerenciadorSimulacao.hpp"
#include "telas/TelaInicial.hpp"
#include "telas/TelaSimulacao.hpp"

int main()
{
    // inicializa gerenciador gráfico
    GerenciadorGrafico gerenciadorGrafico(1280, 720, "SO");

    if (!gerenciadorGrafico.inicializar())
        return -1;

    TelaInicial   telaInicial;
    TelaSimulacao telaSimulacao;

    while (!gerenciadorGrafico.janelaDeveFechar()) {
        gerenciadorGrafico.processarEventos();
        gerenciadorGrafico.iniciarFrame();

        if (!telaInicial.isSimulacaoIniciada()) {
            telaInicial.desenhar();
        } 
        else {
            GerenciadorSimulacao* g = GerenciadorSimulacao::getInstance();
            bool voltar = telaSimulacao.desenhar(g);

            PedidoExportacao pedido = telaSimulacao.consumirPedidoExportacao();
            if (!pedido.caminho.empty()) {
                GanttChart exportador;
                ImVec2 tamanho = exportador.calcularTamanhoCompleto(g);
                bool sucesso = gerenciadorGrafico.exportarPNG(
                    pedido.caminho,
                    tamanho,
                    [&](ImDrawList* dl, ImVec2 origem) {
                        exportador.desenharCompleto(dl, g, origem);
                    });
                telaSimulacao.registrarResultadoExportacao(pedido.caminho, sucesso);
            }

            if (voltar)
                telaInicial.resetar();
        }

        gerenciadorGrafico.renderizar();
    }

    GerenciadorSimulacao::resetar();
    return 0;
}
