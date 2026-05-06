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
            if (voltar)
                telaInicial.resetar();
        }

        gerenciadorGrafico.renderizar();
    }

    GerenciadorTarefa::resetar();
    return 0;
}
