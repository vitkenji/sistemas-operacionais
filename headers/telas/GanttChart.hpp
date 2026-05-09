#pragma once
// GanttChart.hpp
// Componente responsável por desenhar o Gráfico de Gantt dentro de um child
// scrollável do ImGui (req. 2.1 a 2.5).
//
// O Gráfico de Gantt exibe:
// - Uma linha por tarefa (ordenadas por ID crescente de baixo para cima — req. 2.5)
// - Cor da tarefa quando em execução; contorno cinza quando pronta; preto quando suspensa (req. 2.1)
// - Label "Cx" na célula indicando qual CPU está executando a tarefa (req. 2.1 / req. 3)
// - Ícone ▼ (triângulo verde) no tick de chegada da tarefa (req. 2.2)
// - Ícone ⚑ (bandeira vermelha) no tick de término da tarefa (req. 2.2)
// - Ícone ◆ (diamante amarelo) quando o empate foi resolvido por sorteio (req. 4.3 item 4)
// - Seção de CPUs abaixo das tarefas: destaca períodos de CPU desligada (req. 1.2)
// - Legenda explicativa de todos os elementos gráficos
//
// Uso: instanciar como membro de uma tela e chamar desenhar(g) dentro de qualquer
// janela ImGui a cada frame.

#include "gerenciadores/GerenciadorTarefa.hpp"

class GanttChart {
public:
    // Desenha o gráfico completo (tarefas + CPUs + legenda) usando o estado
    // atual do GerenciadorTarefa. Deve ser chamado entre ImGui::Begin/End.
    void desenhar(GerenciadorTarefa* g);

private:
    // Dimensões das células do gráfico (em pixels lógicos do ImGui)
    static constexpr float CELL_W    = 24.f;  // largura de cada coluna de tick
    static constexpr float CELL_H    = 34.f;  // altura de cada linha de tarefa
    static constexpr float LABEL_W   = 52.f;  // largura da coluna de rótulos (ex.: "T3")
    static constexpr float HEADER_H  = 22.f;  // altura do cabeçalho de números de tick
    static constexpr float CPU_ROW_H = 18.f;  // altura de cada linha na seção de CPUs
};
