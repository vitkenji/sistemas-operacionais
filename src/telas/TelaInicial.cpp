// TelaInicial.cpp
// Implementação da tela de configuração inicial.
// Exibe um formulário de importação de arquivo e, após o carregamento bem-sucedido,
// mostra os parâmetros editáveis para que o usuário possa ajustá-los antes
// de iniciar a simulação (req. 3.2 — sugerir e permitir sobrescrever defaults).

#include "telas/TelaInicial.hpp"
#include "gerenciadores/GerenciadorTarefa.hpp"
#include "imgui.h"

#include <string>

// Converte uma string hex RGB ("F0E0D0") para ImVec4 normalizado [0,1].
// Usado para exibir a cor da tarefa no swatch da tabela de pré-visualização.
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

// Nomes dos algoritmos exibidos no ComboBox (índice == algoritmoIdx)
static const char* kAlgoritmos[] = { "PRIOP", "SRTF" };

TelaInicial::TelaInicial()
{
    caminhoArquivo[0] = '\0';
    tentouCarregar    = false;
    simulacaoIniciada = false;

    // Valores padrão visíveis ao usuário (req. 3.2)
    algoritmoIdx    = 0;   // PRIOP
    quantumEditado  = 2;
    qtdeCpusEditado = 2;
}

bool TelaInicial::isSimulacaoIniciada() const { return simulacaoIniciada; }

void TelaInicial::resetar()
{
    ultimaConfig      = ConfigSimulacao{};
    tentouCarregar    = false;
    simulacaoIniciada = false;
    // Volta aos defaults ao retornar para a tela de configuração
    algoritmoIdx    = 0;
    quantumEditado  = 2;
    qtdeCpusEditado = 2;
    GerenciadorTarefa::resetar();
}

void TelaInicial::processarImportacao()
{
    ultimaConfig   = CarregadorConfig::carregar(caminhoArquivo);
    tentouCarregar = true;

    // Ao carregar com sucesso, propaga os valores do arquivo para os campos editáveis,
    // permitindo que o usuário ainda os ajuste antes de iniciar (req. 3.2).
    if (ultimaConfig.valida) {
        algoritmoIdx    = (ultimaConfig.algoritmo == "srtf") ? 1 : 0;
        quantumEditado  = ultimaConfig.quantum;
        qtdeCpusEditado = ultimaConfig.qtde_cpus;
    }
}

void TelaInicial::desenhar()
{
    ImGui::SetNextWindowSize(ImVec2(760, 580), ImGuiCond_FirstUseEver);
    ImGui::Begin("Simulador de SO - Configuracao");

    if (!ultimaConfig.valida)
        desenharFormulario();
    else
        desenharResultado();

    ImGui::End();
}

// Painel exibido antes de qualquer arquivo ser carregado.
// Mostra o campo de caminho, o botão de importação e a seção de defaults editáveis.
void TelaInicial::desenharFormulario()
{
    ImGui::Text("Importe o arquivo de configuracao (.txt) para iniciar a simulacao.");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Caminho do arquivo:");
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputText("##caminho", caminhoArquivo, IM_ARRAYSIZE(caminhoArquivo));
    ImGui::Spacing();

    if (ImGui::Button("Importar", ImVec2(120, 0)))
        processarImportacao();

    if (tentouCarregar && !ultimaConfig.valida) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f),
                           "Erro: %s", ultimaConfig.erroMensagem.c_str());
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Seção de parâmetros padrão — req. 3.2:
    // O usuário pode ajustar os valores mesmo antes de importar um arquivo.
    // Se um arquivo for importado, esses campos serão sobrescritos com os valores do arquivo
    // (mas continuam editáveis após o carregamento, na tela seguinte).
    ImGui::Spacing();
    ImGui::TextDisabled("Parametros padrao (serao sobrescritos pelo arquivo importado):");
    ImGui::Spacing();

    ImGui::SetNextItemWidth(120.f);
    ImGui::Combo("Algoritmo##default", &algoritmoIdx, kAlgoritmos, 2);
    ImGui::SameLine();
    ImGui::TextDisabled("(PRIOP ou SRTF)");

    ImGui::SetNextItemWidth(80.f);
    ImGui::InputInt("Quantum##default", &quantumEditado);
    if (quantumEditado < 1) quantumEditado = 1;

    ImGui::SetNextItemWidth(80.f);
    ImGui::InputInt("CPUs##default", &qtdeCpusEditado);
    if (qtdeCpusEditado < 2) qtdeCpusEditado = 2;

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextDisabled("Formato do arquivo:");
    ImGui::TextDisabled("  Linha 1: algoritmo;quantum;qtde_cpus");
    ImGui::TextDisabled("  Linhas seguintes: id;cor;ingresso;duracao;prioridade[;eventos]");
    ImGui::TextDisabled("  Algoritmos: SRTF, PRIOP  |  qtde_cpus >= 2");
}

// Painel exibido após um arquivo ser carregado com sucesso.
// Mostra os parâmetros editáveis (pre-preenchidos com o arquivo) e a tabela de tarefas.
// O usuário pode ajustar qualquer parâmetro antes de clicar em "Iniciar Simulacao".
void TelaInicial::desenharResultado()
{
    ImGui::TextColored(ImVec4(0.4f, 1.f, 0.4f, 1.f), "Configuracao carregada com sucesso!");
    ImGui::TextDisabled("Ajuste os parametros abaixo antes de iniciar (req. 3.2).");
    ImGui::Separator();
    ImGui::Spacing();

    // Campos editáveis: valores vêm do arquivo mas o usuário pode mudar (req. 3.2)
    ImGui::SetNextItemWidth(120.f);
    ImGui::Combo("Algoritmo##edit", &algoritmoIdx, kAlgoritmos, 2);

    ImGui::SameLine(0.f, 20.f);
    ImGui::SetNextItemWidth(80.f);
    ImGui::InputInt("Quantum##edit", &quantumEditado);
    if (quantumEditado < 1) quantumEditado = 1;

    ImGui::SameLine(0.f, 20.f);
    ImGui::SetNextItemWidth(80.f);
    ImGui::InputInt("CPUs##edit", &qtdeCpusEditado);
    if (qtdeCpusEditado < 2) qtdeCpusEditado = 2;

    ImGui::Spacing();
    ImGui::Text("Tarefas carregadas: %d", static_cast<int>(ultimaConfig.tarefas.size()));
    ImGui::Spacing();

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
            ImGui::TextDisabled("(Projeto B)");
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
        // Aplica os parâmetros editados pelo usuário sobre a configuração do arquivo (req. 3.2)
        ultimaConfig.algoritmo  = (algoritmoIdx == 1) ? "srtf" : "priop";
        ultimaConfig.quantum    = quantumEditado;
        ultimaConfig.qtde_cpus  = qtdeCpusEditado;

        GerenciadorTarefa::configurar(ultimaConfig);
        simulacaoIniciada = true;
    }
}
