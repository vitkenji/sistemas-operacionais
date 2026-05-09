// CarregadorConfig.cpp
// Implementação do parser do arquivo de configuração (req. 3.3).
// Veja CarregadorConfig.hpp para o formato esperado e decisões de design.

#include "config/CarregadorConfig.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

// Abre e faz o parse linha a linha do arquivo de configuração.
// Retorna config.valida = false com uma mensagem de erro em qualquer falha.
ConfigSimulacao CarregadorConfig::carregar(const std::string& caminho)
{
    ConfigSimulacao config;
    std::ifstream arquivo(caminho);

    if (!arquivo.is_open()) {
        config.erroMensagem = "Nao foi possivel abrir: " + caminho;
        return config;
    }

    std::string linha;
    bool primeiraLinha = true;
    int  numeroLinha   = 0;

    while (std::getline(arquivo, linha)) {
        ++numeroLinha;

        // Remove \r para compatibilidade com arquivos CRLF (Windows)
        if (!linha.empty() && linha.back() == '\r')
            linha.pop_back();

        if (linha.empty())
            continue;

        std::vector<std::string> campos = split(linha, ';');

        if (primeiraLinha) {
            // Linha 1: algoritmo_escalonamento;quantum;qtde_cpus
            if (campos.size() < 3) {
                config.erroMensagem = "Linha 1 invalida: esperado algoritmo;quantum;qtde_cpus";
                return config;
            }
            // Normaliza para minúsculas para comparação case-insensitive (req. 3.3.2)
            config.algoritmo = toLower(campos[0]);
            try {
                config.quantum    = std::stoi(campos[1]);
                config.qtde_cpus  = std::stoi(campos[2]);
            } catch (...) {
                config.erroMensagem = "Linha 1: quantum e qtde_cpus devem ser inteiros";
                return config;
            }
            if (config.qtde_cpus < 2) {
                config.erroMensagem = "qtde_cpus deve ser >= 2 (minimo exigido pelo enunciado)";
                return config;
            }
            primeiraLinha = false;

        } else {
            // Linhas 2+: id;cor;ingresso;duracao;prioridade[;lista_eventos]
            if (campos.size() < 5) {
                config.erroMensagem = "Linha " + std::to_string(numeroLinha) +
                    " invalida: esperado id;cor;ingresso;duracao;prioridade[;lista_eventos]";
                return config;
            }
            try {
                int         id         = std::stoi(campos[0]);
                std::string cor        = campos[1];
                int         ingresso   = std::stoi(campos[2]);
                int         duracao    = std::stoi(campos[3]);
                int         prioridade = std::stoi(campos[4]);

                // Lista de eventos é opcional; tratada no Projeto B (req. 3.3.3)
                std::vector<int> eventos;
                if (campos.size() > 5 && !campos[5].empty())
                    eventos = parseListaEventos(campos[5]);

                config.tarefas.emplace_back(id, cor, ingresso, duracao, prioridade, eventos);
            } catch (...) {
                config.erroMensagem = "Linha " + std::to_string(numeroLinha) +
                    ": erro ao converter campos numericos";
                return config;
            }
        }
    }

    if (primeiraLinha) {
        config.erroMensagem = "Arquivo vazio ou sem linha de cabecalho";
        return config;
    }
    if (config.tarefas.empty()) {
        config.erroMensagem = "Nenhuma tarefa encontrada no arquivo";
        return config;
    }

    config.valida = true;
    return config;
}

// Converte 's' para minúsculas usando ::tolower caractere a caractere.
std::string CarregadorConfig::toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

// Divide 's' pelo delimitador 'delim' retornando todos os tokens, incluindo vazios.
// Usado tanto para as linhas (';') quanto para a lista de eventos (',').
std::vector<std::string> CarregadorConfig::split(const std::string& s, char delim)
{
    std::vector<std::string> resultado;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delim))
        resultado.push_back(token);
    return resultado;
}

// Converte uma string de eventos separados por vírgula em vetor de inteiros.
// Tokens inválidos (não numéricos) são ignorados silenciosamente — tolerância
// a erros de formatação na lista de eventos, que é opcional (req. 3.3.3).
std::vector<int> CarregadorConfig::parseListaEventos(const std::string& s)
{
    std::vector<int> eventos;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (!token.empty()) {
            try { eventos.push_back(std::stoi(token)); }
            catch (...) {}
        }
    }
    return eventos;
}
