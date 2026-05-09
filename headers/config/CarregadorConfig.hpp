#pragma once
// CarregadorConfig.hpp
// Lê e valida o arquivo de configuração da simulação (req. 3.3).
//
// Formato esperado (plain text):
//   Linha 1: algoritmo_escalonamento;quantum;qtde_cpus
//   Linhas 2+: id;cor;ingresso;duracao;prioridade[;lista_eventos]
//
// Decisões de design:
// - Toda a lógica de parse é estática; não há estado entre chamadas.
// - O campo 'algoritmo' é normalizado para minúsculas para comparação
//   case-insensitive (req. 3.3.2).
// - Erros retornam ConfigSimulacao com valida=false e uma mensagem descritiva,
//   evitando exceções e facilitando exibição na UI.

#include "tarefa/tarefa.hpp"
#include <string>
#include <vector>

// Resultado completo da leitura do arquivo de configuração.
// Verificar 'valida' antes de usar os demais campos.
struct ConfigSimulacao {
    std::string algoritmo;      // sempre em minúsculas: "srtf" ou "priop"
    int quantum   = 2;          // valor padrão exibido na UI (req. 3.2)
    int qtde_cpus = 2;          // mínimo exigido pelo enunciado
    std::vector<Tarefa> tarefas;
    bool valida = false;
    std::string erroMensagem;   // não vazio apenas quando valida == false
};

class CarregadorConfig {
public:
    // Lê e valida o arquivo no caminho informado.
    // Retorna config.valida = false em caso de qualquer erro de formato ou I/O.
    static ConfigSimulacao carregar(const std::string& caminho);

private:
    // Converte todos os caracteres de 's' para minúsculas (usado no algoritmo).
    static std::string              toLower(std::string s);
    // Divide 's' pelo delimitador e retorna os tokens (incluindo vazios).
    static std::vector<std::string> split(const std::string& s, char delim);
    // Converte uma lista de eventos separada por vírgula ("5,10,15") em vetor de ints.
    static std::vector<int>         parseListaEventos(const std::string& s);
};
