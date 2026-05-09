#include "config/CarregadorConfig.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

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
            // Formato linha 1: algoritmo_escalonamento;quantum;qtde_cpus
            if (campos.size() < 3) {
                config.erroMensagem = "Linha 1 invalida: esperado algoritmo;quantum;qtde_cpus";
                return config;
            }
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
            // Formato linhas 2+: id;cor;ingresso;duracao;prioridade[;lista_eventos]
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

std::string CarregadorConfig::toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

std::vector<std::string> CarregadorConfig::split(const std::string& s, char delim)
{
    std::vector<std::string> resultado;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delim))
        resultado.push_back(token);
    return resultado;
}

std::vector<int> CarregadorConfig::parseListaEventos(const std::string& s)
{
    std::vector<int> eventos;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (!token.empty()) {
            try { eventos.push_back(std::stoi(token)); }
            catch (...) {}  // ignora tokens inválidos silenciosamente
        }
    }
    return eventos;
}
