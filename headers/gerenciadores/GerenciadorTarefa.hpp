#pragma once
#include "config/CarregadorConfig.hpp"
#include "escalonadores/Escalonador.hpp"

#include <map>
#include <vector>

class GerenciadorTarefa
{
private:
    static GerenciadorTarefa* instance;

    std::vector<Tarefa>          listaTarefas;
    std::map<EstadoTarefa, int>  contagemEstados;
    Escalonador*                 pEscalonador;
    int                          qtde_cpus;
    int                          quantum;

    explicit GerenciadorTarefa(const ConfigSimulacao& config);
    Escalonador* criarEscalonador(const std::string& tipo);
    void         atualizarContagemEstados(int tempoAtual);

public:
    ~GerenciadorTarefa();

    // Reconfigura (ou cria) o gerenciador com os dados do arquivo de config.
    // Deve ser chamado antes de getInstance().
    static void              configurar(const ConfigSimulacao& config);
    static void              resetar();
    static GerenciadorTarefa* getInstance();

    void avancaTempo(int tempoAtual);
    int  getQuantidadeEstado(EstadoTarefa estado) const;
    int  getQtdeCpus() const;
    int  getQuantum()  const;
};
