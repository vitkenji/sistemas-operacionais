// GanttChart.cpp
// Implementação do Gráfico de Gantt (req. 2.1 a 2.5).
// Veja GanttChart.hpp para a descrição completa dos elementos visuais.
//
// O desenho usa exclusivamente ImGui::DrawList (primitivas 2D imediatas):
// retângulos preenchidos/delineados para os estados, e formas geométricas
// simples para os ícones de eventos. Essa abordagem não requer texturas
// nem assets externos, mantendo a portabilidade do projeto.

#include "telas/GanttChart.hpp"
#include "imgui.h"

#include <algorithm>
#include <cstdio>
#include <string>

// ── Helper: hex → ImVec4 ──────────────────────────────────────────────────────

// Converte uma string RGB hexadecimal ("F0E0D0") em ImVec4 normalizado [0,1].
// Usado para obter a cor de preenchimento de cada tarefa no Gantt.
static ImVec4 hexParaImVec4Gantt(const std::string& hex)
{
    if (hex.size() < 6) return ImVec4(1.f, 1.f, 1.f, 1.f);
    try {
        unsigned r = std::stoul(hex.substr(0, 2), nullptr, 16);
        unsigned g = std::stoul(hex.substr(2, 2), nullptr, 16);
        unsigned b = std::stoul(hex.substr(4, 2), nullptr, 16);
        return ImVec4(r / 255.f, g / 255.f, b / 255.f, 1.f);
    } catch (...) { return ImVec4(1.f, 1.f, 1.f, 1.f); }
}

// Retorna o estado de uma tarefa a partir de um snapshot do sistema.
// Se a tarefa não estiver no snapshot (ex.: ainda não chegou), retorna Nova.
static EstadoTarefa estadoDaTarefa(const EstadoSistema& snap, int id)
{
    for (const auto& ts : snap.tarefas)
        if (ts.id == id) return ts.estado;
    return EstadoTarefa::Nova;
}

// ── Ícones ────────────────────────────────────────────────────────────────────

// ▼ Triângulo verde apontando para baixo — marca chegada da tarefa no sistema (req. 2.2).
// Desenhado no topo da célula do tick de ingresso da tarefa.
static void iconChegada(ImDrawList* dl, float cx, float top)
{
    constexpr float hw = 5.f;
    constexpr float h  = 8.f;
    dl->AddTriangleFilled(
        ImVec2(cx - hw, top), ImVec2(cx + hw, top), ImVec2(cx, top + h),
        IM_COL32(50, 210, 50, 240));
    // Contorno mais escuro para destaque sobre fundos claros
    dl->AddTriangle(
        ImVec2(cx - hw, top), ImVec2(cx + hw, top), ImVec2(cx, top + h),
        IM_COL32(20, 130, 20, 255));
}

// ⚑ Mastro + bandeira triangular vermelha — marca o término da tarefa (req. 2.2).
// Posicionado na borda do último tick de execução.
static void iconTermino(ImDrawList* dl, float x, float rowY, float cellH)
{
    dl->AddLine(ImVec2(x, rowY + 2.f), ImVec2(x, rowY + cellH - 2.f),
                IM_COL32(255, 70, 70, 255), 1.5f);
    float mid = rowY + cellH * 0.35f;
    dl->AddTriangleFilled(
        ImVec2(x,      rowY + 4.f),
        ImVec2(x + 8.f, mid),
        ImVec2(x,      mid + (mid - rowY - 4.f)),
        IM_COL32(255, 70, 70, 220));
}

// ◆ Diamante amarelo — indica que o empate foi resolvido por sorteio (req. 4.3 item 4).
// Desenhado no canto superior direito da célula da tarefa sorteada.
static void iconSorteio(ImDrawList* dl, float cx, float cy)
{
    constexpr float r = 4.f;
    dl->AddQuadFilled(
        ImVec2(cx,     cy - r), ImVec2(cx + r, cy),
        ImVec2(cx,     cy + r), ImVec2(cx - r, cy),
        IM_COL32(255, 215, 0, 230));
    dl->AddQuad(
        ImVec2(cx,     cy - r), ImVec2(cx + r, cy),
        ImVec2(cx,     cy + r), ImVec2(cx - r, cy),
        IM_COL32(200, 155, 0, 255));
}

// ── Legenda (req. 3 / req. 2.1 / req. 2.2 / req. 4.3) ────────────────────────

// Desenha uma linha de legenda abaixo do canvas do Gantt.
// Cada item exibe um ícone amostral + rótulo textual para todos os tipos de elemento
// gráfico usados no Gráfico de Gantt.
static void desenharLegenda()
{
    ImDrawList* dl      = ImGui::GetWindowDrawList();
    constexpr float S   = 14.f;   // lado do ícone amostral
    constexpr float GI  =  4.f;   // gap ícone → texto
    constexpr float GT  = 12.f;   // gap entre itens

    // Retângulo sólido + borda opcional para amostras de cor
    auto retang = [&](ImU32 fill, ImU32 border, const char* label) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        if (fill)   dl->AddRectFilled(p, ImVec2(p.x + S, p.y + S), fill,   2.f);
        if (border) dl->AddRect      (p, ImVec2(p.x + S, p.y + S), border, 2.f);
        ImGui::Dummy(ImVec2(S, S));
        ImGui::SameLine(0.f, GI);
        ImGui::TextUnformatted(label);
        ImGui::SameLine(0.f, GT);
    };

    // ▼ Triângulo verde (chegada) — amostra inline na legenda
    auto chegada = [&](const char* label) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        float cx = p.x + S * 0.5f, top = p.y + 1.f;
        dl->AddTriangleFilled(ImVec2(cx-5,top), ImVec2(cx+5,top), ImVec2(cx,top+8),
                              IM_COL32(50,210,50,240));
        dl->AddTriangle      (ImVec2(cx-5,top), ImVec2(cx+5,top), ImVec2(cx,top+8),
                              IM_COL32(20,130,20,255));
        ImGui::Dummy(ImVec2(S, S));
        ImGui::SameLine(0.f, GI);
        ImGui::TextUnformatted(label);
        ImGui::SameLine(0.f, GT);
    };

    // ⚑ Bandeira vermelha (término) — amostra inline na legenda
    auto termino = [&](const char* label) {
        ImVec2 p  = ImGui::GetCursorScreenPos();
        float x   = p.x + 3.f, y0 = p.y, y1 = p.y + S;
        float mid = y0 + (y1 - y0) * 0.35f;
        dl->AddLine(ImVec2(x, y0), ImVec2(x, y1), IM_COL32(255,70,70,255), 1.5f);
        dl->AddTriangleFilled(ImVec2(x, y0+2), ImVec2(x+7, mid),
                              ImVec2(x, mid + (mid-y0-2)), IM_COL32(255,70,70,220));
        ImGui::Dummy(ImVec2(S, S));
        ImGui::SameLine(0.f, GI);
        ImGui::TextUnformatted(label);
        ImGui::SameLine(0.f, GT);
    };

    // ◆ Diamante amarelo (sorteio) — amostra inline na legenda
    auto sorteio = [&](const char* label) {
        ImVec2 p  = ImGui::GetCursorScreenPos();
        float cx  = p.x + S * 0.5f, cy = p.y + S * 0.5f, r = 5.f;
        dl->AddQuadFilled(ImVec2(cx,cy-r), ImVec2(cx+r,cy),
                          ImVec2(cx,cy+r), ImVec2(cx-r,cy),
                          IM_COL32(255,215,0,230));
        dl->AddQuad      (ImVec2(cx,cy-r), ImVec2(cx+r,cy),
                          ImVec2(cx,cy+r), ImVec2(cx-r,cy),
                          IM_COL32(200,155,0,255));
        ImGui::Dummy(ImVec2(S, S));
        ImGui::SameLine(0.f, GI);
        ImGui::TextUnformatted(label);
        ImGui::SameLine(0.f, GT);
    };

    ImGui::Spacing();
    ImGui::TextDisabled("Legenda:");
    ImGui::SameLine(0.f, 8.f);

    retang(IM_COL32(255,165, 50,200), 0,                          "Executando (cor da tarefa)");
    retang(0,                          IM_COL32(110,110,110,200), "Pronta");
    retang(IM_COL32(  0,  0,  0,230), IM_COL32( 80, 80, 80,200), "Suspensa");
    chegada("Chegada");
    termino("Termino");
    sorteio("Sorteio");
    retang(IM_COL32( 90, 20, 20,210), 0,                          "CPU desligada");

    ImGui::NewLine();
}

// ── Ponto de entrada ──────────────────────────────────────────────────────────

// Desenha o Gráfico de Gantt completo: cabeçalho de ticks, linhas de tarefas
// com ícones, seção de CPUs e legenda.
void GanttChart::desenhar(GerenciadorTarefa* g)
{
    if (!g) return;

    const auto& tarefasRaw = g->getTarefas();
    if (tarefasRaw.empty()) {
        ImGui::TextDisabled("Nenhuma tarefa carregada.");
        return;
    }

    int tickMax      = g->getTickAtual();
    const auto& hist = g->getHistorico();   // hist[0..tickMax]
    int nCPUs        = g->getQtdeCpus();

    // Ordena por ID decrescente: ID menor fica na última linha (mais próxima do eixo X).
    // No ImGui, Y cresce para baixo, então a "última linha" é a mais baixa visualmente,
    // satisfazendo o req. 2.5 ("tarefa com ID menor é a mais próxima do eixo X").
    std::vector<const Tarefa*> tarefas;
    tarefas.reserve(tarefasRaw.size());
    for (const auto& t : tarefasRaw) tarefas.push_back(&t);
    std::sort(tarefas.begin(), tarefas.end(),
              [](const Tarefa* a, const Tarefa* b){ return a->getID() > b->getID(); });

    int   nRows  = (int)tarefas.size();
    float totalW = LABEL_W + std::max(1, tickMax) * CELL_W;
    // Altura total: cabeçalho + linhas de tarefas + separador (6 px) + linhas de CPUs
    float totalH = HEADER_H + nRows * CELL_H + 6.f + nCPUs * CPU_ROW_H;

    // Limita a altura do child a 420 px para não ocupar toda a janela em simulações longas
    float childH = std::min(totalH + 20.f, 420.f);
    ImGui::BeginChild("##gantt_canvas", ImVec2(0.f, childH), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 origin  = ImGui::GetCursorScreenPos();   // ajustado pelo scroll atual
    ImGui::Dummy(ImVec2(totalW, totalH));            // dimensiona a área de scroll

    // Fundo escuro do canvas
    dl->AddRectFilled(origin, ImVec2(origin.x + totalW, origin.y + totalH),
                      IM_COL32(28, 28, 28, 255));

    // ── Pré-computa tick de término por linha ──────────────────────────────────
    // termTick[row] = primeiro tick em que a tarefa aparece como Terminada; -1 se não chegou lá.
    // Usado para posicionar o ícone ⚑ sem percorrer o histórico duas vezes.
    std::vector<int> termTick(nRows, -1);
    for (int row = 0; row < nRows && tickMax > 0; ++row) {
        int id = tarefas[row]->getID();
        for (int t = 1; t <= tickMax && termTick[row] == -1; ++t)
            if (estadoDaTarefa(hist[(size_t)t], id) == EstadoTarefa::Terminada)
                termTick[row] = t;
    }

    // ── Cabeçalho de ticks ─────────────────────────────────────────────────────
    // Exibe números nos ticks 1, múltiplos de 5 e o último, para não poluir.
    for (int t = 1; t <= tickMax; ++t) {
        float x = origin.x + LABEL_W + (t - 1) * CELL_W;
        if (t == 1 || t % 5 == 0 || t == tickMax) {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "%d", t);
            dl->AddText(ImVec2(x + 2.f, origin.y + 4.f),
                        IM_COL32(180, 180, 180, 255), buf);
        }
        dl->AddLine(ImVec2(x, origin.y + HEADER_H - 5.f),
                    ImVec2(x, origin.y + HEADER_H),
                    IM_COL32(90, 90, 90, 200));
    }

    // ── Linhas de tarefas ──────────────────────────────────────────────────────
    for (int row = 0; row < nRows; ++row) {
        const Tarefa* task = tarefas[row];
        float rowY = origin.y + HEADER_H + row * CELL_H;
        int   id   = task->getID();

        // Fundo alternado para facilitar a leitura em simulações com muitas tarefas
        ImU32 rowBg = (row % 2 == 0) ? IM_COL32(45, 45, 45, 255)
                                      : IM_COL32(38, 38, 38, 255);
        dl->AddRectFilled(ImVec2(origin.x, rowY),
                          ImVec2(origin.x + totalW, rowY + CELL_H), rowBg);

        char label[16];
        std::snprintf(label, sizeof(label), "T%d", id);
        dl->AddText(ImVec2(origin.x + 5.f, rowY + (CELL_H - 13.f) * 0.5f),
                    IM_COL32(220, 220, 220, 255), label);

        if (tickMax == 0) continue;

        // ── Células por tick ───────────────────────────────────────────────────
        for (int t = 1; t <= tickMax; ++t) {
            const EstadoSistema& snap = hist[(size_t)t];
            EstadoTarefa estado = estadoDaTarefa(snap, id);

            // Descobre qual CPU está executando esta tarefa neste tick (para o label)
            int cpuId = -1;
            for (const auto& [cid, tid] : snap.alocacaoCPU)
                if (tid == id) { cpuId = cid; break; }

            float cellX = origin.x + LABEL_W + (t - 1) * CELL_W;
            ImVec2 p0(cellX + 1.f, rowY + 2.f);
            ImVec2 p1(cellX + CELL_W - 1.f, rowY + CELL_H - 2.f);

            switch (estado) {
                case EstadoTarefa::Execucao: {
                    // Preenchimento com a cor da tarefa + label da CPU (req. 2.1)
                    ImU32 fill = ImGui::ColorConvertFloat4ToU32(
                                     hexParaImVec4Gantt(task->getCorHex()));
                    dl->AddRectFilled(p0, p1, fill, 3.f);
                    if (cpuId >= 0) {
                        char cpuLbl[8];
                        std::snprintf(cpuLbl, sizeof(cpuLbl), "C%d", cpuId);
                        dl->AddText(ImVec2(p0.x + 2.f, rowY + (CELL_H - 13.f) * 0.5f),
                                    IM_COL32(0, 0, 0, 200), cpuLbl);
                    }
                    break;
                }
                case EstadoTarefa::Pronta:
                    // Ausência de cor + contorno cinza (req. 2.1)
                    dl->AddRect(p0, p1, IM_COL32(110, 110, 110, 120), 3.f);
                    break;
                case EstadoTarefa::Suspensa:
                    // Preto (req. 2.1)
                    dl->AddRectFilled(p0, p1, IM_COL32(0, 0, 0, 230), 3.f);
                    break;
                default: break;
            }

            // ◆ Ícone de sorteio — canto superior direito da célula (req. 4.3 item 4)
            for (int sid : snap.sorteadas) {
                if (sid == id) {
                    iconSorteio(dl, cellX + CELL_W - 5.f, rowY + 5.f);
                    break;
                }
            }
        }

        // ▼ Ícone de chegada — topo da célula do tick de ingresso (req. 2.2).
        // Tarefas com ingresso=0 são tratadas como ingresso=1 (já presentes no início).
        int ingresso = task->getIngresso();
        if (ingresso <= tickMax) {
            int visCol = std::max(1, ingresso);
            float cx = origin.x + LABEL_W + (visCol - 1) * CELL_W + CELL_W * 0.5f;
            iconChegada(dl, cx, rowY + 1.f);
        }

        // ⚑ Ícone de término — borda esquerda da coluna do tick em que a tarefa terminou (req. 2.2).
        // termTick é o 1º tick com estado Terminada; o último tick de execução foi termTick-1,
        // então a borda direita desse bloco coincide com a borda esquerda da coluna termTick.
        if (termTick[row] != -1) {
            float flagX = origin.x + LABEL_W + (termTick[row] - 1) * CELL_W;
            iconTermino(dl, flagX, rowY, CELL_H);
        }
    }

    // ── Grade da seção de tarefas ──────────────────────────────────────────────
    float taskBottom = origin.y + HEADER_H + nRows * CELL_H;

    for (int row = 0; row <= nRows; ++row) {
        float y = origin.y + HEADER_H + row * CELL_H;
        dl->AddLine(ImVec2(origin.x, y), ImVec2(origin.x + totalW, y),
                    IM_COL32(65, 65, 65, 220));
    }
    for (int t = 0; t <= tickMax; ++t) {
        float x = origin.x + LABEL_W + t * CELL_W;
        dl->AddLine(ImVec2(x, origin.y + HEADER_H), ImVec2(x, taskBottom),
                    IM_COL32(65, 65, 65, 220));
    }
    // Separador vertical entre rótulos e células
    dl->AddLine(ImVec2(origin.x + LABEL_W, origin.y),
                ImVec2(origin.x + LABEL_W, taskBottom),
                IM_COL32(100, 100, 100, 255));

    // ── Seção de CPUs (req. 1.2) ───────────────────────────────────────────────
    // Mostra períodos em que cada CPU esteve desligada (marcados em vermelho escuro).
    // Linha separadora entre a seção de tarefas e a seção de CPUs
    float cpuTop = taskBottom + 6.f;
    dl->AddLine(ImVec2(origin.x, cpuTop - 3.f),
                ImVec2(origin.x + totalW, cpuTop - 3.f),
                IM_COL32(80, 80, 80, 220));

    for (int c = 0; c < nCPUs; ++c) {
        float cpuY = cpuTop + c * CPU_ROW_H;

        dl->AddRectFilled(ImVec2(origin.x, cpuY),
                          ImVec2(origin.x + totalW, cpuY + CPU_ROW_H),
                          IM_COL32(32, 32, 32, 255));

        char cpuLbl[16];
        std::snprintf(cpuLbl, sizeof(cpuLbl), "CPU%d", c);
        dl->AddText(ImVec2(origin.x + 3.f, cpuY + (CPU_ROW_H - 13.f) * 0.5f),
                    IM_COL32(150, 150, 150, 255), cpuLbl);

        // Destaca em vermelho escuro os ticks em que a CPU esteve desligada
        for (int t = 1; t <= tickMax; ++t) {
            const EstadoSistema& snap = hist[(size_t)t];
            auto itL = snap.cpuLigada.find(c);
            bool ligada = (itL == snap.cpuLigada.end()) ? true : itL->second;

            if (!ligada) {
                float bx0 = origin.x + LABEL_W + (t - 1) * CELL_W + 1.f;
                float bx1 = bx0 + CELL_W - 2.f;
                dl->AddRectFilled(ImVec2(bx0, cpuY + 1.f),
                                  ImVec2(bx1, cpuY + CPU_ROW_H - 1.f),
                                  IM_COL32(90, 20, 20, 210));
                dl->AddText(ImVec2(bx0 + 1.f, cpuY + (CPU_ROW_H - 13.f) * 0.5f),
                            IM_COL32(200, 90, 90, 255), "OFF");
            }
        }

        // Grade vertical e borda inferior da linha de CPU
        for (int t = 0; t <= tickMax; ++t)
            dl->AddLine(ImVec2(origin.x + LABEL_W + t * CELL_W, cpuY),
                        ImVec2(origin.x + LABEL_W + t * CELL_W, cpuY + CPU_ROW_H),
                        IM_COL32(50, 50, 50, 180));
        dl->AddLine(ImVec2(origin.x, cpuY + CPU_ROW_H),
                    ImVec2(origin.x + totalW, cpuY + CPU_ROW_H),
                    IM_COL32(55, 55, 55, 200));
        dl->AddLine(ImVec2(origin.x + LABEL_W, cpuY),
                    ImVec2(origin.x + LABEL_W, cpuY + CPU_ROW_H),
                    IM_COL32(80, 80, 80, 200));
    }

    // Placeholder quando nenhum tick foi simulado ainda
    if (tickMax == 0) {
        dl->AddText(
            ImVec2(origin.x + LABEL_W + 8.f, origin.y + HEADER_H + 10.f),
            IM_COL32(120, 120, 120, 200),
            "Use 'Avancar >>' para iniciar a simulacao");
    }

    ImGui::EndChild();

    desenharLegenda();
}
