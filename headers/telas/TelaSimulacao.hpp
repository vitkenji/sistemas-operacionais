#pragma once
#include "gerenciadores/GerenciadorTarefa.hpp"
#include "telas/GanttChart.hpp"

class TelaSimulacao {
public:
    // Retorna true quando o usuário clica em "Voltar para configuração"
    bool desenhar(GerenciadorTarefa* g);

    // Retorna o caminho do PNG pedido (não vazio) e limpa o pedido.
    // Deve ser chamado após desenhar() e antes de renderizar().
    std::string consumirPedidoExportacao();

private:
    GanttChart  gantt;
    bool        flagExportarPNG  = false;
    int         framesNotificacao = 0;
    std::string ultimaExportacao;

    void desenharPainelCPUs(GerenciadorTarefa* g);
    void desenharTabelaTarefas(GerenciadorTarefa* g);
    void desenharGantt(GerenciadorTarefa* g);
    void desenharControles(GerenciadorTarefa* g, bool& voltar);
};
