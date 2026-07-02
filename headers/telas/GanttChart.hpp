#pragma once
#include "gerenciadores/GerenciadorSimulacao.hpp"
#include "imgui.h"

// desenha o Gráfico de Gantt.
class GanttChart {
public:
    void desenhar(GerenciadorSimulacao* g);

    ImVec2 calcularTamanhoCompleto(GerenciadorSimulacao* g) const;
    void   desenharCompleto(ImDrawList* dl, GerenciadorSimulacao* g, ImVec2 origin) const;

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
    static constexpr float EXPORT_MIN_W = 720.f;

    ImVec2 calcularTamanhoTabela(GerenciadorSimulacao* g) const;
    float  calcularAlturaLegenda(float largura) const;
    float  desenharLegendaEm(ImDrawList* dl, ImVec2 origin, float largura) const;
    void   desenharTabelaEm(ImDrawList* dl, GerenciadorSimulacao* g, ImVec2 origin) const;

    float ultimaMinX = 0.f, ultimaMinY = 0.f;
    float ultimaMaxX = 0.f, ultimaMaxY = 0.f;
};
