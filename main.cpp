#include "gerenciadores/GerenciadorGrafico.hpp"
#include "gerenciadores/GerenciadorTarefa.hpp"
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
            GerenciadorTarefa* g = GerenciadorTarefa::getInstance();
            bool voltar = telaSimulacao.desenhar(g);

            std::string pngPath = telaSimulacao.consumirPedidoExportacao();
            if (!pngPath.empty())
                gerenciadorGrafico.pedirCaptura(pngPath);

            if (voltar)
                telaInicial.resetar();
        }

        gerenciadorGrafico.renderizar();
    }

    GerenciadorTarefa::resetar();
    return 0;
}
