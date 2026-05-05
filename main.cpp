#include "gerenciadores/GerenciadorGrafico.hpp"
#include "gerenciadores/GerenciadorTarefa.hpp"
#include "tarefa/tarefa.hpp"
#include "imgui.h" // Apenas para o ImGui::ShowDemoWindow();
#include <iostream>
#include <vector>

int main() {
    GerenciadorGrafico gerenciadorGrafico(1280, 720, "");

    if (!gerenciadorGrafico.inicializar()) {
        return -1;
    }

    // TODO

    GerenciadorTarefa* gerenciadorTarefa = GerenciadorTarefa::getInstance("priop");

    std::vector<int> eventosTarefa1, eventosTarefa2, eventosTarefa3;

    Tarefa tarefa1(1, 0, 5, 1, eventosTarefa1);
    Tarefa tarefa2(2, 1, 3, 2, eventosTarefa2);
    Tarefa tarefa3(3, 2, 4, 3, eventosTarefa3);

    tarefa1.registrarEstadoNoTempo(0, EstadoTarefa::Nova);
    tarefa2.registrarEstadoNoTempo(0, EstadoTarefa::Nova);
    tarefa3.registrarEstadoNoTempo(0, EstadoTarefa::Nova);

    gerenciadorTarefa->adicionarTarefa(tarefa1);
    gerenciadorTarefa->adicionarTarefa(tarefa2);
    gerenciadorTarefa->adicionarTarefa(tarefa3);

    int contadorLoop = 1;
    int tempoAtual = 1;

    // loop principal
    while (!gerenciadorGrafico.janelaDeveFechar()) {
        
        if (contadorLoop % 100 == 0) {
            gerenciadorTarefa->avancaTempo(tempoAtual);
            tempoAtual++;
        }
        contadorLoop++;

        gerenciadorGrafico.processarEventos();
        gerenciadorGrafico.iniciarFrame();

        ImGui::ShowDemoWindow(); 

        //mostra na tela
        gerenciadorGrafico.renderizar();
    }

    return 0;
}