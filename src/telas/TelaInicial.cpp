#include "telas/TelaInicial.hpp"
#include "gerenciadores/GerenciadorTarefa.hpp"
#include "imgui.h"
#include <cstring>
#include <string>

static ImVec4 hexParaImVec4(const std::string& hex)
{
    if (hex.size() < 6) return ImVec4(1.f, 1.f, 1.f, 1.f);
    try {
        unsigned r = std::stoul(hex.substr(0, 2), nullptr, 16);
        unsigned g = std::stoul(hex.substr(2, 2), nullptr, 16);
        unsigned b = std::stoul(hex.substr(4, 2), nullptr, 16);
        return ImVec4(r / 255.f, g / 255.f, b / 255.f, 1.f);
    } catch (...) { return ImVec4(1.f, 1.f, 1.f, 1.f); }
}

TelaInicial::TelaInicial()
{
    caminhoArquivo[0] = '\0';
    tentouCarregar    = false;
    simulacaoIniciada = false;
}

bool TelaInicial::isSimulacaoIniciada() const { return simulacaoIniciada; }

void TelaInicial::resetar()
{
    ultimaConfig      = ConfigSimulacao{};
    tentouCarregar    = false;
    simulacaoIniciada = false;
    GerenciadorTarefa::resetar();
}

void TelaInicial::processarImportacao()
{
    ultimaConfig   = CarregadorConfig::carregar(caminhoArquivo);
    tentouCarregar = true;
}

void TelaInicial::carregarExemplo(const char* caminho)
{
    std::strncpy(caminhoArquivo, caminho, sizeof(caminhoArquivo) - 1);
    caminhoArquivo[sizeof(caminhoArquivo) - 1] = '\0';
    processarImportacao();
}

void TelaInicial::desenhar()
{
    // obtém dimensões atuais da janela para cobrir toda a tela
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    // desativa decorações para que a janela funcione como tela inteira
    ImGui::Begin("Simulador de SO - Configuracao", nullptr,
                 ImGuiWindowFlags_NoTitleBar  |
                 ImGuiWindowFlags_NoResize    |
                 ImGuiWindowFlags_NoMove      |
                 ImGuiWindowFlags_NoCollapse  |
                 ImGuiWindowFlags_NoBringToFrontOnFocus);

    // exibe formulário enquanto não há configuração válida; após carga, mostra resultado
    if (!ultimaConfig.valida)
        desenharFormulario();
    else
        desenharResultado();

    ImGui::End();
}

//tela para importar arquivo
void TelaInicial::desenharFormulario()
{
    ImGui::Text("Importe o arquivo de configuracao (.txt)");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Caminho do arquivo:");
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputText("##caminho", caminhoArquivo, IM_ARRAYSIZE(caminhoArquivo));
    ImGui::Spacing();

    if (ImGui::Button("Importar", ImVec2(120, 0)))
        processarImportacao();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Ou carregue um exemplo:");
    ImGui::Spacing();

    if (ImGui::Button("Exemplo 1 - PRIOP basico", ImVec2(240, 0)))
        carregarExemplo("exemplos/01_priop_basico.txt");

    if (ImGui::Button("Exemplo 2 - SRTF basico", ImVec2(240, 0)))
        carregarExemplo("exemplos/02_srtf_basico.txt");

    if (ImGui::Button("Exemplo 3 - Sorteio", ImVec2(240, 0)))
        carregarExemplo("exemplos/03_sorteio.txt");

    if (ImGui::Button("Exemplo 4 - PRIOP complexo", ImVec2(240, 0)))
        carregarExemplo("exemplos/04_priop_complexo.txt");

    if (tentouCarregar && !ultimaConfig.valida) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f),
                           "Erro: %s", ultimaConfig.erroMensagem.c_str());
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextDisabled("  Linha 1: [algoritmo=priop][;quantum=1][;qtde_cpus=2]");
    ImGui::TextDisabled("  Linhas seguintes: id;cor;ingresso;duracao[;prioridade=0][;eventos]");
    ImGui::TextDisabled("  Algoritmos: SRTF, PRIOP  |  qtde_cpus >= 2");
    ImGui::TextDisabled("  * Digite ;; na primeira linha para usar");
    ImGui::TextDisabled("  valores padrão sugeridos: priop;1;2");
}

//tela após importar arquivo
void TelaInicial::desenharResultado()
{
    ImGui::TextColored(ImVec4(0.4f, 1.f, 0.4f, 1.f), "Configuracao carregada com sucesso!");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Algoritmo : %s", ultimaConfig.algoritmo.c_str());
    ImGui::Text("Quantum   : %d tick(s)", ultimaConfig.quantum);
    ImGui::Text("CPUs      : %d", ultimaConfig.qtde_cpus);
    ImGui::Text("Tarefas   : %d", static_cast<int>(ultimaConfig.tarefas.size()));
    ImGui::Spacing();

    ImGui::Text("Tarefas carregadas:");
    if (ImGui::BeginTable("tabelaTarefas", 6,
            ImGuiTableFlags_Borders    |
            ImGuiTableFlags_RowBg      |
            ImGuiTableFlags_ScrollY    |
            ImGuiTableFlags_SizingFixedFit,
            ImVec2(0, 200)))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("ID",         ImGuiTableColumnFlags_WidthFixed, 40.f);
        ImGui::TableSetupColumn("Cor",        ImGuiTableColumnFlags_WidthFixed, 90.f);
        ImGui::TableSetupColumn("Ingresso",   ImGuiTableColumnFlags_WidthFixed, 70.f);
        ImGui::TableSetupColumn("Duracao",    ImGuiTableColumnFlags_WidthFixed, 70.f);
        ImGui::TableSetupColumn("Prioridade", ImGuiTableColumnFlags_WidthFixed, 80.f);
        ImGui::TableSetupColumn("Eventos",    ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (const auto& t : ultimaConfig.tarefas) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", t.getID());

            ImGui::TableSetColumnIndex(1);
            ImVec4 cor = hexParaImVec4(t.getCorHex());
            std::string btnId = "##cor" + std::to_string(t.getID());
            ImGui::ColorButton(btnId.c_str(), cor,
                               ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel,
                               ImVec2(18, 18));
            ImGui::SameLine();
            ImGui::Text("#%s", t.getCorHex().c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%d", t.getIngresso());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%d", t.getDuracao());

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%d", t.getPrioridade());

            ImGui::TableSetColumnIndex(5);
            ImGui::TextDisabled(".");
        }

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Carregar outro arquivo", ImVec2(200, 0))) {
        ultimaConfig   = ConfigSimulacao{};
        tentouCarregar = false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Iniciar Simulacao >>", ImVec2(180, 0))) {
        GerenciadorTarefa::configurar(ultimaConfig);
        simulacaoIniciada = true;
    }
}
