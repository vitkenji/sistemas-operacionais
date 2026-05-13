#pragma once
#include "gerenciadores/GerenciadorTarefa.hpp"
#include "telas/GanttChart.hpp"

class TelaSimulacao {
public:
    // true quando user clica em "Voltar para configuração"
    bool desenhar(GerenciadorTarefa* g);

    // retorna caminho do PNG pedido e limpa o pedido.
    // chamado após desenhar() e antes de renderizar().
    std::string consumirPedidoExportacao();

private:
    GanttChart  gantt;
    bool        flagExportarPNG  = false;
    bool        exportacaoConcluida = false; // flag para exibir mensagem 
    std::string ultimaExportacao; //caminho do arquivo exportado

    void desenharPainelCPUs(GerenciadorTarefa* g);
    void desenharTabelaTarefas(GerenciadorTarefa* g);
    void desenharGantt(GerenciadorTarefa* g);
    void desenharControles(GerenciadorTarefa* g, bool& voltar);
};
