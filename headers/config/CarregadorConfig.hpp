#pragma once
#include "tarefa/tarefa.hpp"
#include <string>
#include <vector>

// Resultado da leitura do arquivo de configuração.
// Verificar 'valida' antes de usar os demais campos.
struct ConfigSimulacao {
    std::string algoritmo;      // sempre em minúsculas: "srtf" ou "priop"
    int quantum   = 1;
    int qtde_cpus = 2;
    std::vector<Tarefa> tarefas;
    bool valida = false;
    std::string erroMensagem;
};

class CarregadorConfig {
public:
    // Lê e valida o arquivo de configuração no formato definido pelo enunciado.
    // Retorna config.valida = false em caso de qualquer erro.
    static ConfigSimulacao carregar(const std::string& caminho);

private:
    static std::string              toLower(std::string s);
    static std::vector<std::string> split(const std::string& s, char delim);
    // Eventos são separados por vírgula: "5,10,15" → {5, 10, 15}
    static std::vector<int>         parseListaEventos(const std::string& s);
};
