#pragma once
#include "gerenciadores/GerenciadorSimulacao.hpp"
#include "telas/GanttChart.hpp"
#include <string>

// dados devolvidos por consumirPedidoExportacao()
struct PedidoExportacao {
    std::string caminho;
    float minX = 0, minY = 0, maxX = 0, maxY = 0;
};

class TelaSimulacao {
public:
    // true quando user clica em "Voltar para configuração"
    bool desenhar(GerenciadorSimulacao* g);

    // retorna pedido de exportação e limpa o estado interno.
    // caminho vazio = nenhum pedido pendente.
    PedidoExportacao consumirPedidoExportacao();

private:
    GanttChart  gantt;
    bool        flagExportarPNG     = false;
    bool        exportacaoConcluida = false;
    std::string ultimaExportacao;
    float       ganttMinX = 0, ganttMinY = 0, ganttMaxX = 0, ganttMaxY = 0;

    void desenharPainelCPUs(GerenciadorSimulacao* g);
    void desenharTabelaTarefas(GerenciadorSimulacao* g);
    void desenharGantt(GerenciadorSimulacao* g);
    void desenharControles(GerenciadorSimulacao* g, bool& voltar);
};
