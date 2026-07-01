#include "telas/GanttChart.hpp"
#include "imgui.h"
#include <algorithm>
#include <cstdio>
#include <string>

// hex -> ImVec4 
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

static const SnapshotTarefa* snapshotDaTarefa(const EstadoSistema& snap, const std::string& id)
{
    for (const auto& ts : snap.tarefas)
        if (ts.id == id) return &ts;
    return nullptr;
}

// retorna  estado de tarefa a partir do snapshot
static EstadoTarefa estadoDaTarefa(const EstadoSistema& snap, const std::string& id)
{
    if (const SnapshotTarefa* ts = snapshotDaTarefa(snap, id))
        return ts->estado;
    // nova, se não encontrada
    return EstadoTarefa::Nova;
}

static MotivoSuspensao motivoDaTarefa(const EstadoSistema& snap, const std::string& id)
{
    if (const SnapshotTarefa* ts = snapshotDaTarefa(snap, id))
        return ts->motivoSuspensao;
    return MotivoSuspensao::Nenhum;
}

// triângulo verde apontando para baixo — marca chegada da tarefa
static void iconChegada(ImDrawList* dl, float cx, float top)
{
    constexpr float hw = 5.f;
    constexpr float h  = 8.f;
    dl->AddTriangleFilled(
        ImVec2(cx - hw, top), ImVec2(cx + hw, top), ImVec2(cx, top + h),
        IM_COL32(50, 210, 50, 240));
    dl->AddTriangle(
        ImVec2(cx - hw, top), ImVec2(cx + hw, top), ImVec2(cx, top + h),
        IM_COL32(20, 130, 20, 255));
}

// mastro + bandeira triangular vermelha — marca término da tarefa
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

// diamante amarelo — sorteio
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

static void iconMutexAcao(ImDrawList* dl, float cx, float cy, TipoEventoGantt tipo)
{
    ImU32 fill = (tipo == TipoEventoGantt::SolicitarMutex)
        ? IM_COL32(70, 190, 255, 235)
        : IM_COL32(255, 120, 80, 235);
    const char* label = (tipo == TipoEventoGantt::SolicitarMutex) ? "L" : "U";

    dl->AddCircleFilled(ImVec2(cx, cy), 5.f, fill);
    dl->AddCircle(ImVec2(cx, cy), 5.f, IM_COL32(20, 20, 20, 220));
    dl->AddText(ImVec2(cx - 3.f, cy - 6.f), IM_COL32(0, 0, 0, 230), label);
}

static void iconIOAcao(ImDrawList* dl, float cx, float cy, TipoEventoGantt tipo)
{
    if (tipo == TipoEventoGantt::InicioIO) {
        dl->AddRectFilled(ImVec2(cx - 5.f, cy - 5.f), ImVec2(cx + 5.f, cy + 5.f),
                          IM_COL32(120, 180, 255, 235), 2.f);
        dl->AddRect(ImVec2(cx - 5.f, cy - 5.f), ImVec2(cx + 5.f, cy + 5.f),
                    IM_COL32(20, 60, 140, 230), 2.f);
        dl->AddText(ImVec2(cx - 3.f, cy - 6.f), IM_COL32(0, 0, 0, 230), "I");
        return;
    }

    dl->AddTriangleFilled(ImVec2(cx, cy - 6.f), ImVec2(cx + 6.f, cy + 5.f),
                          ImVec2(cx - 6.f, cy + 5.f), IM_COL32(255, 235, 90, 240));
    dl->AddTriangle(ImVec2(cx, cy - 6.f), ImVec2(cx + 6.f, cy + 5.f),
                    ImVec2(cx - 6.f, cy + 5.f), IM_COL32(150, 120, 20, 255));
    dl->AddText(ImVec2(cx - 3.f, cy - 4.f), IM_COL32(0, 0, 0, 230), "!");
}

static void preencherSuspensaMutex(ImDrawList* dl, ImVec2 p0, ImVec2 p1)
{
    dl->AddRectFilled(p0, p1, IM_COL32(20, 20, 20, 235), 3.f);
    for (float x = p0.x - (p1.y - p0.y); x < p1.x; x += 6.f) {
        dl->AddLine(ImVec2(x, p1.y), ImVec2(x + (p1.y - p0.y), p0.y),
                    IM_COL32(120, 120, 120, 180), 1.f);
    }
    dl->AddRect(p0, p1, IM_COL32(150, 150, 150, 180), 3.f);
}

static void preencherSuspensaIO(ImDrawList* dl, ImVec2 p0, ImVec2 p1)
{
    dl->AddRectFilled(p0, p1, IM_COL32(20, 45, 95, 235), 3.f);
    for (float y = p0.y + 4.f; y < p1.y; y += 6.f) {
        dl->AddLine(ImVec2(p0.x + 2.f, y), ImVec2(p1.x - 2.f, y),
                    IM_COL32(135, 185, 255, 175), 1.f);
    }
    dl->AddRect(p0, p1, IM_COL32(120, 170, 250, 180), 3.f);
}

// desenha linha de legenda imediatamente abaixo do child do Gantt.
// cada item: pequeno ícone amostral + rótulo textual
static void desenharLegenda()
{
    ImDrawList* dl      = ImGui::GetWindowDrawList();
    constexpr float S   = 14.f;   // lado do ícone amostral
    constexpr float GI  =  4.f;   // gap ícone → texto
    constexpr float GT  = 12.f;   // gap entre itens

    // retângulo + borda opcional
    auto retang = [&](ImU32 fill, ImU32 border, const char* label) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        if (fill)   dl->AddRectFilled(p, ImVec2(p.x + S, p.y + S), fill,   2.f);
        if (border) dl->AddRect      (p, ImVec2(p.x + S, p.y + S), border, 2.f);
        ImGui::Dummy(ImVec2(S, S));
        ImGui::SameLine(0.f, GI);
        ImGui::TextUnformatted(label);
        ImGui::SameLine(0.f, GT);
    };

    // triângulo verde (chegada)
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

    // bandeira vermelha (término)
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

    // diamante amarelo (sorteio)
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

    auto mutexAcao = [&](TipoEventoGantt tipo, const char* label) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        iconMutexAcao(dl, p.x + S * 0.5f, p.y + S * 0.5f, tipo);
        ImGui::Dummy(ImVec2(S, S));
        ImGui::SameLine(0.f, GI);
        ImGui::TextUnformatted(label);
        ImGui::SameLine(0.f, GT);
    };

    auto suspensaMutex = [&](const char* label) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        preencherSuspensaMutex(dl, p, ImVec2(p.x + S, p.y + S));
        ImGui::Dummy(ImVec2(S, S));
        ImGui::SameLine(0.f, GI);
        ImGui::TextUnformatted(label);
        ImGui::SameLine(0.f, GT);
    };

    auto ioAcao = [&](TipoEventoGantt tipo, const char* label) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        iconIOAcao(dl, p.x + S * 0.5f, p.y + S * 0.5f, tipo);
        ImGui::Dummy(ImVec2(S, S));
        ImGui::SameLine(0.f, GI);
        ImGui::TextUnformatted(label);
        ImGui::SameLine(0.f, GT);
    };

    auto suspensaIO = [&](const char* label) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        preencherSuspensaIO(dl, p, ImVec2(p.x + S, p.y + S));
        ImGui::Dummy(ImVec2(S, S));
        ImGui::SameLine(0.f, GI);
        ImGui::TextUnformatted(label);
        ImGui::SameLine(0.f, GT);
    };

    ImGui::Spacing();
    ImGui::SameLine(0.f, 8.f);

    retang(IM_COL32(255,165, 50,200), 0,                          "Executando (cor da tarefa)");
    retang(0,                          IM_COL32(110,110,110,200), "Pronta");
    retang(IM_COL32(  0,  0,  0,230), IM_COL32( 80, 80, 80,200), "Suspensa");
    suspensaMutex("Suspensa por mutex");
    suspensaIO("Suspensa por E/S");
    chegada("Chegada");
    termino("Termino");
    sorteio("Sorteio");
    mutexAcao(TipoEventoGantt::SolicitarMutex, "Solicita mutex");
    mutexAcao(TipoEventoGantt::LiberarMutex, "Libera mutex");
    ioAcao(TipoEventoGantt::InicioIO, "Inicio E/S");
    ioAcao(TipoEventoGantt::IRQ, "IRQ");
    retang(IM_COL32( 90, 20, 20,210), 0,                          "CPU desligada");

    ImGui::NewLine();
}

// ponto de entrada

void GanttChart::desenhar(GerenciadorSimulacao* g)
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

    // ID menor fica na linha mais baixa
    std::vector<const Tarefa*> tarefas;
    tarefas.reserve(tarefasRaw.size());
    for (const auto& t : tarefasRaw) tarefas.push_back(&t);
    std::sort(tarefas.begin(), tarefas.end(),
              [](const Tarefa* a, const Tarefa* b){ return a->getID() > b->getID(); });

    int   nRows  = (int)tarefas.size();
    float totalW = LABEL_W + std::max(1, tickMax) * CELL_W;
    // Altura: cabeçalho + linhas de tarefas + separador (6 px) + linhas de CPUs
    float totalH = HEADER_H + nRows * CELL_H + 6.f + nCPUs * CPU_ROW_H;

    float childH = std::min(totalH + 20.f, 420.f);
    ImVec2 regionStart = ImGui::GetCursorScreenPos();
    ImGui::BeginChild("##gantt_canvas", ImVec2(0.f, childH), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 origin  = ImGui::GetCursorScreenPos();   // já ajustado pelo scroll atual
    ImGui::Dummy(ImVec2(totalW, totalH));            // dimensiona a área de scroll

    // fundo
    dl->AddRectFilled(origin, ImVec2(origin.x + totalW, origin.y + totalH),
                      IM_COL32(28, 28, 28, 255));

    // pre-computa tick de término por linha
    // termTick[row] = primeiro tick em que a tarefa aparece como Terminada; -1 se ainda não.
    std::vector<int> termTick(nRows, -1);
    for (int row = 0; row < nRows && tickMax > 0; ++row) {
        std::string id = tarefas[row]->getID();
        for (int t = 1; t <= tickMax && termTick[row] == -1; ++t)
            if (estadoDaTarefa(hist[(size_t)t], id) == EstadoTarefa::Terminada)
                termTick[row] = t;
    }

    // cabecalho de ticks
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

    // linhas de tarefas
    for (int row = 0; row < nRows; ++row) {
        const Tarefa* task = tarefas[row];
        float rowY = origin.y + HEADER_H + row * CELL_H;
        std::string id = task->getID();

        // fundo alternado
        ImU32 rowBg = (row % 2 == 0) ? IM_COL32(45, 45, 45, 255)
                                      : IM_COL32(38, 38, 38, 255);
        dl->AddRectFilled(ImVec2(origin.x, rowY),
                          ImVec2(origin.x + totalW, rowY + CELL_H), rowBg);

        dl->AddText(ImVec2(origin.x + 5.f, rowY + (CELL_H - 13.f) * 0.5f),
                    IM_COL32(220, 220, 220, 255), id.c_str());

        if (tickMax == 0) continue;

        // células por tick
        for (int t = 1; t <= tickMax; ++t) {
            const EstadoSistema& snap = hist[(size_t)t];
            EstadoTarefa estado = estadoDaTarefa(snap, id);

            int cpuId = -1;
            for (const auto& [cid, tid] : snap.alocacaoCPU)
                if (tid == id) { cpuId = cid; break; }

            float cellX = origin.x + LABEL_W + (t - 1) * CELL_W;
            ImVec2 p0(cellX + 1.f, rowY + 2.f);
            ImVec2 p1(cellX + CELL_W - 1.f, rowY + CELL_H - 2.f);

            switch (estado) {
                case EstadoTarefa::Execucao: {
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
                    dl->AddRect(p0, p1, IM_COL32(110, 110, 110, 120), 3.f);
                    break;
                case EstadoTarefa::Suspensa:
                    if (motivoDaTarefa(snap, id) == MotivoSuspensao::Mutex)
                        preencherSuspensaMutex(dl, p0, p1);
                    else if (motivoDaTarefa(snap, id) == MotivoSuspensao::EntradaSaida)
                        preencherSuspensaIO(dl, p0, p1);
                    else
                        dl->AddRectFilled(p0, p1, IM_COL32(0, 0, 0, 230), 3.f);
                    break;
                default: break;
            }

            // Ícone de sorteio ◆ — no canto superior direito da célula
            for (const std::string& sid : snap.sorteadas) {
                if (sid == id) {
                    iconSorteio(dl, cellX + CELL_W - 5.f, rowY + 5.f);
                    break;
                }
            }

            int eventoIdx = 0;
            for (const auto& evento : snap.eventos) {
                if (evento.tarefaId == id) {
                    float cx = cellX + 6.f + eventoIdx * 9.f;
                    float cy = rowY + CELL_H - 8.f;
                    if (evento.tipo == TipoEventoGantt::InicioIO ||
                        evento.tipo == TipoEventoGantt::IRQ)
                        iconIOAcao(dl, cx, cy, evento.tipo);
                    else
                        iconMutexAcao(dl, cx, cy, evento.tipo);
                    eventoIdx++;
                }
            }
        }

        // icone de chegada: ingresso=0 aparece à direita do label; demais, na coluna do ingresso
        int ingresso = task->getIngresso();
        if (ingresso == 0) {
            float cx = origin.x + LABEL_W - 6.f;
            iconChegada(dl, cx, rowY + 1.f);
        } else if (ingresso <= tickMax) {
            int visCol = std::max(1, ingresso);
            float cx = origin.x + LABEL_W + (visCol - 1) * CELL_W + CELL_W * 0.5f;
            iconChegada(dl, cx, rowY + 1.f);
        }

        // icone de término na borda direita do último tick de execução
        // termTick é o 1º tick em estado Terminada; o último tick de execução foi termTick-1,
        // então a borda direita desse bloco coincide com a borda esquerda da coluna termTick.
        if (termTick[row] != -1) {
            float flagX = origin.x + LABEL_W + (termTick[row] - 1) * CELL_W;
            iconTermino(dl, flagX, rowY, CELL_H);
        }
    }

    // grade da seção de tarefas
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
    dl->AddLine(ImVec2(origin.x + LABEL_W, origin.y),
                ImVec2(origin.x + LABEL_W, taskBottom),
                IM_COL32(100, 100, 100, 255));

    // seção de CPUs — mostra períodos em que cada CPU esteve desligada
    // linha separadora entre tarefas e CPUs
    float cpuTop = taskBottom + 6.f;
    dl->AddLine(ImVec2(origin.x, cpuTop - 3.f),
                ImVec2(origin.x + totalW, cpuTop - 3.f),
                IM_COL32(80, 80, 80, 220));

    for (int c = 0; c < nCPUs; ++c) {
        float cpuY = cpuTop + c * CPU_ROW_H;

        // fundo da linha de CPU
        dl->AddRectFilled(ImVec2(origin.x, cpuY),
                          ImVec2(origin.x + totalW, cpuY + CPU_ROW_H),
                          IM_COL32(32, 32, 32, 255));

        char cpuLbl[16];
        std::snprintf(cpuLbl, sizeof(cpuLbl), "CPU%d", c);
        dl->AddText(ImVec2(origin.x + 3.f, cpuY + (CPU_ROW_H - 13.f) * 0.5f),
                    IM_COL32(150, 150, 150, 255), cpuLbl);

        // destaca ticks em que a CPU estava desligada
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

        // grade vertical e borda inferior da linha de CPU
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


    ImGui::EndChild();

    float regionWidth = ImGui::GetItemRectSize().x;
    desenharLegenda();
    ImVec2 regionEnd = ImGui::GetCursorScreenPos();

    ultimaMinX = regionStart.x;
    ultimaMinY = regionStart.y;
    ultimaMaxX = regionStart.x + regionWidth;
    ultimaMaxY = regionEnd.y;
}
