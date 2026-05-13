#pragma once
#include "gerenciadores/GerenciadorTarefa.hpp"

// desenha o Gráfico de Gantt.
class GanttChart {
public:
    void desenhar(GerenciadorTarefa* g);

private:
    static constexpr float CELL_W     = 24.f; 
    static constexpr float CELL_H     = 34.f;
    static constexpr float LABEL_W    = 52.f; 
    static constexpr float HEADER_H   = 22.f; 
    static constexpr float CPU_ROW_H  = 18.f; 
};
