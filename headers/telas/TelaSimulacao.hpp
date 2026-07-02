#pragma once
#include "gerenciadores/GerenciadorSimulacao.hpp"
#include "telas/GanttChart.hpp"
#include <string>

// dados devolvidos por consumirPedidoExportacao()
struct PedidoExportacao {
    std::string caminho;
};

class TelaSimulacao {
public:
    // true quando user clica em "Voltar para configuração"
    bool desenhar(GerenciadorSimulacao* g);

    // retorna pedido de exportação e limpa o estado interno.
    // caminho vazio = nenhum pedido pendente.
    PedidoExportacao consumirPedidoExportacao();
    void registrarResultadoExportacao(const std::string& caminho, bool sucesso);

private:
    GanttChart  gantt;
    bool        flagExportarPNG     = false;
    bool        exportacaoConcluida = false;
    bool        exportacaoFalhou    = false;
    std::string ultimaExportacao;

    void desenharPainelCPUs(GerenciadorSimulacao* g);
    void desenharTabelaTarefas(GerenciadorSimulacao* g);
    void desenharGantt(GerenciadorSimulacao* g);
    void desenharControles(GerenciadorSimulacao* g, bool& voltar);
};
