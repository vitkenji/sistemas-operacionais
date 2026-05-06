#pragma once
#include "gerenciadores/GerenciadorTarefa.hpp"

class TelaSimulacao {
public:
    // Retorna true quando o usuário clica em "Voltar para configuração"
    bool desenhar(GerenciadorTarefa* g);

private:
    void desenharPainelCPUs(GerenciadorTarefa* g);
    void desenharTabelaTarefas(GerenciadorTarefa* g);
    void desenharControles(GerenciadorTarefa* g, bool& voltar);
};
