#pragma once
// TelaSimulacao.hpp
// Tela principal da simulação, exibida após a configuração inicial (req. 1.5 e 2).
//
// Organização da janela:
//   1. Painel de CPUs — estado atual de cada processador (ligada/ociosa/executando qual tarefa)
//   2. Tabela de tarefas — estado, tempos, CPU e controle de edição manual de cada tarefa (req. 3.4)
//   3. Gráfico de Gantt — histórico visual da simulação (req. 2.1 a 2.5)
//   4. Controles — navegar passo-a-passo, executar completo, exportar PNG (req. 1.5 e 2.4)

#include "gerenciadores/GerenciadorTarefa.hpp"
#include "telas/GanttChart.hpp"

class TelaSimulacao {
public:
    // Desenha a janela completa. Retorna true quando o usuário clica em "Voltar".
    bool desenhar(GerenciadorTarefa* g);

    // Retorna o caminho do PNG pedido (string não vazia) e limpa o pedido interno.
    // Deve ser chamado pelo main loop após desenhar() e antes de renderizar(),
    // para que a captura do framebuffer ocorra no mesmo frame.
    std::string consumirPedidoExportacao();

private:
    GanttChart  gantt;
    bool        flagExportarPNG   = false;  // sinaliza pedido pendente de exportação
    int         framesNotificacao = 0;      // contador para exibir mensagem de sucesso por ~3 s
    std::string ultimaExportacao;           // caminho do último PNG exportado

    void desenharPainelCPUs(GerenciadorTarefa* g);
    void desenharTabelaTarefas(GerenciadorTarefa* g);
    void desenharGantt(GerenciadorTarefa* g);
    void desenharControles(GerenciadorTarefa* g, bool& voltar);
};
