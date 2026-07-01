#pragma once
#include "tarefa/tarefa.hpp"
#include <string>
#include <vector>

// parse do txt
// verificar 'valida' antes de usar os campos
struct ConfigSimulacao {
    std::string algoritmo; 
    int quantum   = 1;
    int qtde_cpus = 2;
    int alpha     = 1;
    std::vector<Tarefa> tarefas;
    bool valida = false;
    std::string erroMensagem;
};

class CarregadorConfig {
public:
    // le e valida o arquivo de configuração
    // retorna config.valida = false em caso de qualquer erro.
    static ConfigSimulacao carregar(const std::string& caminho);

private:
    static std::string              toLower(std::string s);
    static std::vector<std::string> split(const std::string& s, char delim);
    static std::string              trim(const std::string& s);
    static bool                     parseAcaoMutex(const std::string& s, AcaoTarefa& acao);
    static bool                     parseAcaoIO(const std::string& s, AcaoTarefa& acao);
};
