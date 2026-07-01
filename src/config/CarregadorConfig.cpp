#include "config/CarregadorConfig.hpp"

#include <algorithm>
#include <cctype>
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

        // remove \r para compatibilidade
        if (!linha.empty() && linha.back() == '\r')
            linha.pop_back();

        if (linha.empty())
            continue;

        std::vector<std::string> campos = split(linha, ';');

        if (primeiraLinha) {
            // [algoritmo=priop][;quantum=1][;qtde_cpus=2][;alpha=1]
            // campo vazio usa o valor padrao; ex: ";;" aplica todos os padroes
            config.algoritmo = toLower(campos[0]);
            if (config.algoritmo.empty()) {
                config.algoritmo = "priop";
            } else if (config.algoritmo != "srtf" && config.algoritmo != "priop" &&
                       config.algoritmo != "priopd" && config.algoritmo != "priopenv") {
                config.erroMensagem = "Algoritmo invalido: '" + campos[0] +
                    "'. Valores aceitos: SRTF, PRIOP, PRIOPD, PRIOPEnv";
                return config;
            }
            try {
                if (campos.size() >= 2 && !campos[1].empty())
                    config.quantum   = std::stoi(campos[1]);
                if (campos.size() >= 3 && !campos[2].empty())
                    config.qtde_cpus = std::stoi(campos[2]);
                if (campos.size() >= 4 && !campos[3].empty())
                    config.alpha     = std::stoi(campos[3]);
            } catch (...) {
                config.erroMensagem = "Linha 1: quantum, qtde_cpus e alpha devem ser inteiros";
                return config;
            }
            if (config.qtde_cpus < 2) {
                config.erroMensagem = "qtde_cpus deve ser >= 2";
                return config;
            }
            if (config.alpha < 0) {
                config.erroMensagem = "alpha deve ser >= 0";
                return config;
            }
            primeiraLinha = false;

        } else {
            // id;cor;ingresso;duracao[;prioridade=0][;acao1][;acao2]...
            if (campos.size() < 4) {
                config.erroMensagem = "Linha " + std::to_string(numeroLinha) +
                    " invalida: esperado id;cor;ingresso;duracao[;prioridade][;acoes]";
                return config;
            }
            std::string id  = campos[0];
            if (id.empty()) {
                config.erroMensagem = "Linha " + std::to_string(numeroLinha) +
                    ": id nao pode ser vazio";
                return config;
            }
            try {
                std::string      cor        = campos[1];
                int              ingresso   = std::stoi(campos[2]);
                int              duracao    = std::stoi(campos[3]);
                int              prioridade = 0;

                if (campos.size() >= 5 && !campos[4].empty())
                    prioridade = std::stoi(campos[4]);

                std::vector<AcaoTarefa> acoes;
                for (size_t i = 5; i < campos.size(); ++i) {
                    std::string token = trim(campos[i]);
                    if (token.empty())
                        continue;

                    std::string tokenLower = toLower(token);
                    if (tokenLower.rfind("io:", 0) == 0)
                        continue;

                    AcaoTarefa acao{};
                    if (parseAcaoMutex(token, acao)) {
                        acoes.push_back(acao);
                    } else if (tokenLower.rfind("ml", 0) == 0 || tokenLower.rfind("mu", 0) == 0) {
                        config.erroMensagem = "Linha " + std::to_string(numeroLinha) +
                            ": acao de mutex invalida: '" + token + "'";
                        return config;
                    }
                }

                config.tarefas.emplace_back(id, cor, ingresso, duracao, prioridade, acoes);
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

std::string CarregadorConfig::trim(const std::string& s)
{
    size_t ini = 0;
    while (ini < s.size() && std::isspace(static_cast<unsigned char>(s[ini])))
        ini++;

    size_t fim = s.size();
    while (fim > ini && std::isspace(static_cast<unsigned char>(s[fim - 1])))
        fim--;

    return s.substr(ini, fim - ini);
}

bool CarregadorConfig::parseAcaoMutex(const std::string& s, AcaoTarefa& acao)
{
    std::string token = trim(s);
    std::string lower = toLower(token);
    if (lower.rfind("ml", 0) != 0 && lower.rfind("mu", 0) != 0)
        return false;

    size_t sep = token.find(':');
    if (sep == std::string::npos || sep <= 2 || sep + 1 >= token.size())
        return false;

    std::string mutexStr = token.substr(2, sep - 2);
    std::string tempoStr = token.substr(sep + 1);

    for (char ch : mutexStr)
        if (!std::isdigit(static_cast<unsigned char>(ch)))
            return false;
    for (char ch : tempoStr)
        if (!std::isdigit(static_cast<unsigned char>(ch)))
            return false;

    acao.tipo = (lower.rfind("ml", 0) == 0)
        ? TipoAcaoTarefa::SolicitarMutex
        : TipoAcaoTarefa::LiberarMutex;
    acao.mutexId = std::stoi(mutexStr);
    acao.tempoRelativo = std::stoi(tempoStr);
    return true;
}
