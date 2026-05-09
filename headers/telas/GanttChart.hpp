#pragma once
#include "gerenciadores/GerenciadorTarefa.hpp"

// Componente que desenha o Gráfico de Gantt dentro de um child scrollável ImGui.
// Requer apenas uma chamada a desenhar(g) dentro de qualquer janela ImGui.
class GanttChart {
public:
    void desenhar(GerenciadorTarefa* g);

private:
    static constexpr float CELL_W     = 24.f;  // largura de cada coluna de tick
    static constexpr float CELL_H     = 34.f;  // altura de cada linha de tarefa
    static constexpr float LABEL_W    = 52.f;  // largura da coluna de rótulos
    static constexpr float HEADER_H   = 22.f;  // altura do cabeçalho de ticks
    static constexpr float CPU_ROW_H  = 18.f;  // altura de cada linha na seção de CPUs
};
