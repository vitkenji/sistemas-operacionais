#include "gerenciadores/gerenciadorGrafico.hpp"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

GerenciadorGrafico::GerenciadorGrafico(int largura, int altura, const std::string& titulo)
    : largura(largura), altura(altura), titulo(titulo), window(nullptr) {}

GerenciadorGrafico::~GerenciadorGrafico() {
    limpar();
}

bool GerenciadorGrafico::inicializar() {
    // inicializa glfw
    if (!glfwInit())
    {
        std::cerr << "erro ao inicializar GLFW" << std::endl;
        return false;
    }

    // define versao do opengl
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // cria a instancia da janela
    window = glfwCreateWindow(largura, altura, titulo.c_str(), NULL, NULL);
    
    // verifica se a janela foi criada
    if (!window)
    {
        std::cerr << "erro ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return false;
    }

    // define a janela como contexto atual
    glfwMakeContextCurrent(window);
    // habilita v-sync
    glfwSwapInterval(1); 

    // inicializa contexto do imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // define tema escuro
    ImGui::StyleColorsDark();

    // vincula imgui ao glfw e opengl
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    return true;
}

bool GerenciadorGrafico::janelaDeveFechar() const {
    // verifica se a janela deve ser fechada
    return glfwWindowShouldClose(window);
}

void GerenciadorGrafico::processarEventos() {
    // processa inputs e eventos do sistema
    glfwPollEvents();
}

void GerenciadorGrafico::iniciarFrame() {
    // prepara o inicio de um novo frame no imgui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GerenciadorGrafico::renderizar() {
    // renderiza interface
    ImGui::Render();

    // atualiza o viewport e limpa a tela de fundo
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    // desenha os dados na tela
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // troca os buffers da janela
    glfwSwapBuffers(window);
}

bool GerenciadorGrafico::exportarPNG(
    const std::string& caminho,
    ImVec2 tamanho,
    const std::function<void(ImDrawList*, ImVec2)>& desenhar)
{
    if (!desenhar || caminho.empty() || tamanho.x <= 0.f || tamanho.y <= 0.f)
        return false;

    int w = std::max(1, (int)std::ceil(tamanho.x));
    int h = std::max(1, (int)std::ceil(tamanho.y));

    unsigned char* texPixels = nullptr;
    int texW = 0, texH = 0, texBpp = 0;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&texPixels, &texW, &texH, &texBpp);
    if (!texPixels || texW <= 0 || texH <= 0 || texBpp != 4) {
        std::cerr << "[GerenciadorGrafico] Falha ao acessar textura da fonte para PNG.\n";
        return false;
    }

    ImDrawList drawList(ImGui::GetDrawListSharedData());
    drawList._ResetForNewFrame();
    drawList.PushClipRect(ImVec2(0.f, 0.f), ImVec2((float)w, (float)h), false);
    desenhar(&drawList, ImVec2(0.f, 0.f));
    drawList.PopClipRect();
    drawList._PopUnusedDrawCmd();

    int stride = w * 4;
    std::vector<unsigned char> out((size_t)(stride * h), 0);

    auto edge = [](const ImVec2& a, const ImVec2& b, float x, float y) {
        return (x - a.x) * (b.y - a.y) - (y - a.y) * (b.x - a.x);
    };

    auto blendPixel = [&](int x, int y, float r, float g, float b, float a) {
        if (x < 0 || y < 0 || x >= w || y >= h || a <= 0.f)
            return;

        size_t off = (size_t)(y * stride + x * 4);
        float dr = out[off + 0] / 255.f;
        float dg = out[off + 1] / 255.f;
        float db = out[off + 2] / 255.f;
        float da = out[off + 3] / 255.f;

        float invA = 1.f - a;
        float oa = a + da * invA;
        float orr = r * a + dr * invA;
        float ogg = g * a + dg * invA;
        float obb = b * a + db * invA;

        out[off + 0] = (unsigned char)(std::max(0.f, std::min(1.f, orr)) * 255.f + 0.5f);
        out[off + 1] = (unsigned char)(std::max(0.f, std::min(1.f, ogg)) * 255.f + 0.5f);
        out[off + 2] = (unsigned char)(std::max(0.f, std::min(1.f, obb)) * 255.f + 0.5f);
        out[off + 3] = (unsigned char)(std::max(0.f, std::min(1.f, oa)) * 255.f + 0.5f);
    };

    auto rasterizarTriangulo = [&](const ImDrawVert& v0,
                                   const ImDrawVert& v1,
                                   const ImDrawVert& v2,
                                   const ImVec4& clip) {
        float minXf = std::min(v0.pos.x, std::min(v1.pos.x, v2.pos.x));
        float minYf = std::min(v0.pos.y, std::min(v1.pos.y, v2.pos.y));
        float maxXf = std::max(v0.pos.x, std::max(v1.pos.x, v2.pos.x));
        float maxYf = std::max(v0.pos.y, std::max(v1.pos.y, v2.pos.y));

        int minX = std::max(0, (int)std::floor(std::max(minXf, clip.x)));
        int minY = std::max(0, (int)std::floor(std::max(minYf, clip.y)));
        int maxX = std::min(w - 1, (int)std::ceil(std::min(maxXf, clip.z)) - 1);
        int maxY = std::min(h - 1, (int)std::ceil(std::min(maxYf, clip.w)) - 1);
        if (minX > maxX || minY > maxY)
            return;

        float area = edge(v0.pos, v1.pos, v2.pos.x, v2.pos.y);
        if (std::fabs(area) < 0.00001f)
            return;

        bool uvConstante =
            std::fabs(v0.uv.x - v1.uv.x) < 0.00001f &&
            std::fabs(v0.uv.y - v1.uv.y) < 0.00001f &&
            std::fabs(v0.uv.x - v2.uv.x) < 0.00001f &&
            std::fabs(v0.uv.y - v2.uv.y) < 0.00001f;

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                float px = x + 0.5f;
                float py = y + 0.5f;

                float e0 = edge(v1.pos, v2.pos, px, py);
                float e1 = edge(v2.pos, v0.pos, px, py);
                float e2 = edge(v0.pos, v1.pos, px, py);

                if ((area > 0.f && (e0 < 0.f || e1 < 0.f || e2 < 0.f)) ||
                    (area < 0.f && (e0 > 0.f || e1 > 0.f || e2 > 0.f)))
                    continue;

                float w0 = e0 / area;
                float w1 = e1 / area;
                float w2 = e2 / area;

                float u = v0.uv.x * w0 + v1.uv.x * w1 + v2.uv.x * w2;
                float v = v0.uv.y * w0 + v1.uv.y * w1 + v2.uv.y * w2;

                auto comp = [](ImU32 col, int shift) {
                    return (float)((col >> shift) & 0xFF) / 255.f;
                };

                float cr = comp(v0.col, IM_COL32_R_SHIFT) * w0 +
                           comp(v1.col, IM_COL32_R_SHIFT) * w1 +
                           comp(v2.col, IM_COL32_R_SHIFT) * w2;
                float cg = comp(v0.col, IM_COL32_G_SHIFT) * w0 +
                           comp(v1.col, IM_COL32_G_SHIFT) * w1 +
                           comp(v2.col, IM_COL32_G_SHIFT) * w2;
                float cb = comp(v0.col, IM_COL32_B_SHIFT) * w0 +
                           comp(v1.col, IM_COL32_B_SHIFT) * w1 +
                           comp(v2.col, IM_COL32_B_SHIFT) * w2;
                float ca = comp(v0.col, IM_COL32_A_SHIFT) * w0 +
                           comp(v1.col, IM_COL32_A_SHIFT) * w1 +
                           comp(v2.col, IM_COL32_A_SHIFT) * w2;

                float tr = 1.f, tg = 1.f, tb = 1.f, ta = 1.f;
                if (!uvConstante) {
                    int tx = std::max(0, std::min(texW - 1, (int)(u * texW)));
                    int ty = std::max(0, std::min(texH - 1, (int)(v * texH)));
                    const unsigned char* texel = texPixels + (ty * texW + tx) * 4;
                    tr = texel[0] / 255.f;
                    tg = texel[1] / 255.f;
                    tb = texel[2] / 255.f;
                    ta = texel[3] / 255.f;
                }

                blendPixel(x, y, tr * cr, tg * cg, tb * cb, ta * ca);
            }
        }
    };

    for (const ImDrawCmd& cmd : drawList.CmdBuffer) {
        if (cmd.UserCallback || cmd.ElemCount == 0)
            continue;

        const ImVec4 clip(
            std::max(0.f, cmd.ClipRect.x),
            std::max(0.f, cmd.ClipRect.y),
            std::min((float)w, cmd.ClipRect.z),
            std::min((float)h, cmd.ClipRect.w));
        if (clip.x >= clip.z || clip.y >= clip.w)
            continue;

        unsigned int idxEnd = cmd.IdxOffset + cmd.ElemCount;
        for (unsigned int idx = cmd.IdxOffset; idx + 2 < idxEnd; idx += 3) {
            ImDrawIdx i0 = drawList.IdxBuffer[(int)idx + 0];
            ImDrawIdx i1 = drawList.IdxBuffer[(int)idx + 1];
            ImDrawIdx i2 = drawList.IdxBuffer[(int)idx + 2];

            const ImDrawVert& v0 = drawList.VtxBuffer[(int)(cmd.VtxOffset + i0)];
            const ImDrawVert& v1 = drawList.VtxBuffer[(int)(cmd.VtxOffset + i1)];
            const ImDrawVert& v2 = drawList.VtxBuffer[(int)(cmd.VtxOffset + i2)];
            rasterizarTriangulo(v0, v1, v2, clip);
        }
    }

    bool temVariacao = false;
    for (size_t i = 4; i + 3 < out.size(); i += 4) {
        if (out[i]     != out[0] ||
            out[i + 1] != out[1] ||
            out[i + 2] != out[2] ||
            out[i + 3] != out[3]) {
            temVariacao = true;
            break;
        }
    }
    if (!temVariacao) {
        std::cerr << "[GerenciadorGrafico] Exportacao PNG gerou imagem vazia: "
                  << caminho << '\n';
        return false;
    }

    bool ok = stbi_write_png(caminho.c_str(), w, h, 4, out.data(), stride) != 0;
    if (!ok)
        std::cerr << "[GerenciadorGrafico] Falha ao salvar PNG: " << caminho << '\n';
    else
        std::cout << "[GerenciadorGrafico] Gantt exportado: " << caminho << '\n';

    return ok;
}

void GerenciadorGrafico::limpar() {
    if (window != nullptr) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
        
        window = nullptr; // garante que nao limpa duas vezes
    }
}
