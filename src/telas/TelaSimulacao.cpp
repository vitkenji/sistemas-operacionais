#include "telas/TelaSimulacao.hpp"
#include "imgui.h"

#include <string>

static const char* nomeEstado(EstadoTarefa e)
{
    switch (e) {
        case EstadoTarefa::Nova:      return "Nova";
        case EstadoTarefa::Pronta:    return "Pronta";
        case EstadoTarefa::Execucao:  return "Em Execucao";
        case EstadoTarefa::Suspensa:  return "Suspensa";
        case EstadoTarefa::Terminada: return "Terminada";
    }
    return "?";
}

static ImVec4 corEstado(EstadoTarefa e)
{
    switch (e) {
        case EstadoTarefa::Nova:      return ImVec4(0.6f, 0.6f, 0.6f, 1.f);
        case EstadoTarefa::Pronta:    return ImVec4(1.0f, 1.0f, 0.3f, 1.f);
        case EstadoTarefa::Execucao:  return ImVec4(0.3f, 1.0f, 0.3f, 1.f);
        case EstadoTarefa::Suspensa:  return ImVec4(0.3f, 0.3f, 0.3f, 1.f);
        case EstadoTarefa::Terminada: return ImVec4(0.5f, 0.5f, 1.0f, 1.f);
    }
    return ImVec4(1, 1, 1, 1);
}

static ImVec4 hexParaImVec4(const std::string& hex)
{
    if (hex.size() < 6) return ImVec4(1, 1, 1, 1);
    try {
        unsigned r = std::stoul(hex.substr(0, 2), nullptr, 16);
        unsigned g = std::stoul(hex.substr(2, 2), nullptr, 16);
        unsigned b = std::stoul(hex.substr(4, 2), nullptr, 16);
        return ImVec4(r / 255.f, g / 255.f, b / 255.f, 1.f);
    } catch (...) { return ImVec4(1, 1, 1, 1); }
}

// ponto de entrada 
bool TelaSimulacao::desenhar(GerenciadorSimulacao* g)
{
    bool voltar = false;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    char titulo[64];
    std::snprintf(titulo, sizeof(titulo), "Simulacao  |  Tick %d###SimWin", g->getTickAtual());
    ImGui::Begin(titulo, nullptr,
                 ImGuiWindowFlags_NoTitleBar  |
                 ImGuiWindowFlags_NoResize    |
                 ImGuiWindowFlags_NoMove      |
                 ImGuiWindowFlags_NoCollapse  |
                 ImGuiWindowFlags_NoBringToFrontOnFocus);

    desenharPainelCPUs(g);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    desenharTabelaTarefas(g);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    desenharGantt(g);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    desenharControles(g, voltar);

    ImGui::End();
    return voltar;
}

// painel de CPUs
void TelaSimulacao::desenharPainelCPUs(GerenciadorSimulacao* g)
{
    ImGui::Text("CPUs  (quantum = %d)", g->getQuantum());
    ImGui::Spacing();

    const auto& cpus    = g->getCPUs();
    const auto& tarefas = g->getTarefas();

    for (const auto& cpu : cpus) {
        // Caixa colorida por status
        ImVec4 cor;
        std::string label;

        if (!cpu.ligada) {
            cor   = ImVec4(0.2f, 0.2f, 0.2f, 1.f);
            label = "DESLIGADA";
        } else if (cpu.tarefaAtualID.empty()) {
            cor   = ImVec4(0.4f, 0.4f, 0.4f, 1.f);
            label = "OCIOSA";
        } else {
            // Encontra cor da tarefa
            cor = ImVec4(0.3f, 0.8f, 0.3f, 1.f);
            for (const auto& t : tarefas) {
                if (t.getID() == cpu.tarefaAtualID) {
                    cor   = hexParaImVec4(t.getCorHex());
                    label = t.getID()
                            + "(restam: " + std::to_string(t.getTempoRestante()) + ")";
                    break;
                }
            }
        }

        char btnId[32];
        std::snprintf(btnId, sizeof(btnId), "CPU %d\n%s##cpu%d",
                      cpu.id, label.c_str(), cpu.id);

        ImGui::PushStyleColor(ImGuiCol_Button,        cor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, cor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  cor);
        ImGui::Button(btnId, ImVec2(110, 55));
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
    }
    ImGui::NewLine();
}

// tabela de tarefas 
void TelaSimulacao::desenharTabelaTarefas(GerenciadorSimulacao* g)
{
    ImGui::Text("Tarefas");
    ImGui::Spacing();

    // nomes dos estados para o combo de edição manual
    static const char* estados[] = {"Nova","Pronta","Em Execucao","Suspensa","Terminada"};

    if (!ImGui::BeginTable("tblTarefas", 10,
            ImGuiTableFlags_Borders    |
            ImGuiTableFlags_RowBg      |
            ImGuiTableFlags_ScrollY    |
            ImGuiTableFlags_SizingFixedFit,
            ImVec2(0, 200)))
        return;

    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("ID",         ImGuiTableColumnFlags_WidthFixed, 30.f);
    ImGui::TableSetupColumn("Cor",        ImGuiTableColumnFlags_WidthFixed, 40.f);
    ImGui::TableSetupColumn("Estado",     ImGuiTableColumnFlags_WidthFixed, 110.f);
    ImGui::TableSetupColumn("T.Rest.",    ImGuiTableColumnFlags_WidthFixed, 55.f);
    ImGui::TableSetupColumn("Quantum",    ImGuiTableColumnFlags_WidthFixed, 55.f);
    ImGui::TableSetupColumn("CPU",        ImGuiTableColumnFlags_WidthFixed, 40.f);
    ImGui::TableSetupColumn("Prioridade", ImGuiTableColumnFlags_WidthFixed, 75.f);
    ImGui::TableSetupColumn("P.Din.",     ImGuiTableColumnFlags_WidthFixed, 55.f);
    ImGui::TableSetupColumn("Ingresso",   ImGuiTableColumnFlags_WidthFixed, 65.f);
    ImGui::TableSetupColumn("Editar",     ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    const auto& cpus = g->getCPUs();

    for (const auto& t : g->getTarefas()) {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", t.getID().c_str());

        ImGui::TableSetColumnIndex(1);
        std::string btnId = "##cor" + t.getID();
        ImGui::ColorButton(btnId.c_str(), hexParaImVec4(t.getCorHex()),
                           ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel,
                           ImVec2(20, 20));

        ImGui::TableSetColumnIndex(2);
        if (t.getEstadoAtual() == EstadoTarefa::Suspensa &&
            t.getMotivoSuspensao() == MotivoSuspensao::Mutex)
            ImGui::TextColored(corEstado(t.getEstadoAtual()), "Susp. Mutex");
        else if (t.getEstadoAtual() == EstadoTarefa::Suspensa &&
                 t.getMotivoSuspensao() == MotivoSuspensao::EntradaSaida)
            ImGui::TextColored(corEstado(t.getEstadoAtual()), "Susp. E/S");
        else
            ImGui::TextColored(corEstado(t.getEstadoAtual()), "%s", nomeEstado(t.getEstadoAtual()));

        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%d", t.getTempoRestante());

        ImGui::TableSetColumnIndex(4);
        if (t.getEstadoAtual() == EstadoTarefa::Execucao)
            ImGui::Text("%d", t.getQuantumRestante());
        else
            ImGui::TextDisabled("--");

        ImGui::TableSetColumnIndex(5);
        {
            int cpuId = -1;
            for (const auto& cpu : cpus)
                if (cpu.tarefaAtualID == t.getID()) { cpuId = cpu.id; break; }
            if (cpuId >= 0) ImGui::Text("CPU%d", cpuId);
            else            ImGui::TextDisabled("--");
        }

        ImGui::TableSetColumnIndex(6);
        ImGui::Text("%d", t.getPrioridade());

        ImGui::TableSetColumnIndex(7);
        ImGui::Text("%d", t.getPrioridadeDinamica());

        ImGui::TableSetColumnIndex(8);
        ImGui::Text("%d", t.getIngresso());

        // edição manual do estado
        ImGui::TableSetColumnIndex(9);
        EstadoTarefa estadoAtual = t.getEstadoAtual();
        bool terminada   = estadoAtual == EstadoTarefa::Terminada;
        bool bloqueaNova = (estadoAtual == EstadoTarefa::Pronta   ||
                            estadoAtual == EstadoTarefa::Execucao ||
                            estadoAtual == EstadoTarefa::Suspensa);
        std::string comboId = "##edit" + t.getID();
        ImGui::SetNextItemWidth(130.f);
        ImGui::BeginDisabled(terminada);
        if (ImGui::BeginCombo(comboId.c_str(), nomeEstado(estadoAtual))) {
            for (int i = 0; i < 5; ++i) {
                ImGui::BeginDisabled(i == 0 && bloqueaNova);
                if (ImGui::Selectable(estados[i], estadoAtual == static_cast<EstadoTarefa>(i)))
                    g->editarEstadoTarefa(t.getID(), static_cast<EstadoTarefa>(i));
                ImGui::EndDisabled();
            }
            ImGui::EndCombo();
        }
        ImGui::EndDisabled();
    }

    ImGui::EndTable();
}

// gantt
void TelaSimulacao::desenharGantt(GerenciadorSimulacao* g)
{
    ImGui::Spacing();
    gantt.desenhar(g);
}

void TelaSimulacao::desenharControles(GerenciadorSimulacao* g, bool& voltar)
{
    // navegação
    if (ImGui::Button("< Voltar", ImVec2(90, 0)))
        voltar = true;

    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();

    ImGui::BeginDisabled(!g->podeRetroceder());
    if (ImGui::Button("<< Retroceder", ImVec2(120, 0)))
        g->retroceder();
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(!g->podeAvancar());
    if (ImGui::Button("Avancar >>", ImVec2(110, 0)))
        g->avancar();
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(!g->podeAvancar());
    if (ImGui::Button("Executar Completo >>|", ImVec2(170, 0)))
        g->executarCompleto();
    ImGui::EndDisabled();

    // status da simulação
    ImGui::Spacing();
    if (g->isSimulacaoCompleta()) {
        ImGui::TextColored(ImVec4(0.4f, 1.f, 0.4f, 1.f),
                           "Simulação concluída.");
    } else {
        ImGui::TextDisabled("Tick %d",
                            g->getTickAtual());
    }

    // exportação de PNG habilitado só ao terminar
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    static char exportPath[256] = "gantt.png";
    ImGui::SetNextItemWidth(220.f);
    ImGui::InputText("##exportpath", exportPath, sizeof(exportPath), ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();

    ImGui::BeginDisabled(!g->isSimulacaoCompleta());
    if (ImGui::Button("Exportar PNG", ImVec2(120, 0))) {
        flagExportarPNG     = true;
        exportacaoConcluida = true;
        ultimaExportacao    = exportPath;
        ganttMinX = gantt.getUltimaMinX();
        ganttMinY = gantt.getUltimaMinY();
        ganttMaxX = gantt.getUltimaMaxX();
        ganttMaxY = gantt.getUltimaMaxY();
    }
    ImGui::EndDisabled();

    if (exportacaoConcluida) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 1.f, 0.4f, 1.f),
                           "Exportado: %s", ultimaExportacao.c_str());
    }
}

// exportacao
PedidoExportacao TelaSimulacao::consumirPedidoExportacao()
{
    if (!flagExportarPNG) return {};
    flagExportarPNG = false;
    return { ultimaExportacao, ganttMinX, ganttMinY, ganttMaxX, ganttMaxY };
}
