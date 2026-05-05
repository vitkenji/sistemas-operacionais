#pragma once
#include <map>
#include <string>
#include <vector>

enum class EstadoTarefa {
    Nova,
    Pronta,
    Execucao,
    Suspensa,
    Terminada
};

class Tarefa
{
private:
    int ID;
    std::string corHex;   // cor RGB em hex, ex: "F0E0D0"
    int ingresso;
    int duracao;
    int prioridade;
    std::vector<int> lista_eventos;
    std::map<int, EstadoTarefa> historicoNoTempo;

public:
    Tarefa(int id, std::string corHex, int ingresso, int duracao,
           int prioridade, std::vector<int> lista_eventos);
    ~Tarefa();

    int         getID()         const;
    std::string getCorHex()     const;
    int         getIngresso()   const;
    int         getDuracao()    const;
    int         getPrioridade() const;

    void        registrarEstadoNoTempo(int instanteTempo, EstadoTarefa novoEstado);
    EstadoTarefa buscarEstadoNoTempo(int instanteTempo) const;
};
