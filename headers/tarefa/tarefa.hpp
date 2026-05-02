#pragma once
#include <map>
#include <vector>

// estados possíveis Tarefa
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
    int ingresso;
    int duracao;
    int prioridade;
    std::vector<int> lista_eventos;
    std::map<int, EstadoTarefa> historicoNoTempo;

public:
    Tarefa(int id, int ingresso, int duracao, int prioridade, std::vector<int> lista_eventos);
    ~Tarefa();

    void registrarEstadoNoTempo(int instanteTempo, EstadoTarefa novoEstado);
    void mostrarEstadoNoTempo(int instanteTempo) const;
    void mostrarLinhaDoTempo() const;
};
