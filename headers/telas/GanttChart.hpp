#pragma once
#include "gerenciadores/GerenciadorTarefa.hpp"

// desenha o Gráfico de Gantt.
class GanttChart {
public:
    void desenhar(GerenciadorTarefa* g);

    // bounding box (coordenadas de tela, pixels lógicos) do último desenhar()
    float getUltimaMinX() const { return ultimaMinX; }
    float getUltimaMinY() const { return ultimaMinY; }
    float getUltimaMaxX() const { return ultimaMaxX; }
    float getUltimaMaxY() const { return ultimaMaxY; }

private:
    static constexpr float CELL_W     = 24.f;
    static constexpr float CELL_H     = 34.f;
    static constexpr float LABEL_W    = 52.f;
    static constexpr float HEADER_H   = 22.f;
    static constexpr float CPU_ROW_H  = 18.f;

    float ultimaMinX = 0.f, ultimaMinY = 0.f;
    float ultimaMaxX = 0.f, ultimaMaxY = 0.f;
};
