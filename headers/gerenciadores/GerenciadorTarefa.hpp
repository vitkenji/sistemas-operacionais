#pragma once
#include "config/CarregadorConfig.hpp"
#include "escalonadores/Escalonador.hpp"
#include "simulacao/CPU.hpp"
#include "simulacao/EstadoSistema.hpp"

#include <map>
#include <vector>

class GerenciadorTarefa {
private:
    static GerenciadorTarefa* instance;

    std::vector<Tarefa>        listaTarefas;
    std::vector<CPU>           cpus;
    Escalonador*               pEscalonador;
    int                        quantum;

    // historico[0] = estado inicial; historico[T] = estado após T ticks
    std::vector<EstadoSistema> historico;
    int                        tickAtual;         // índice no historico
    bool                       simulacaoCompleta;

    explicit GerenciadorTarefa(const ConfigSimulacao& config);
    Escalonador* criarEscalonador(const std::string& tipo);

    // Motor
    void          computarProximoTick();
    void          aplicarEstado(const EstadoSistema& estado);
    EstadoSistema buildSnapshot() const;
    bool          todasTerminadas() const;
    bool          hasTarefaProntaOuExecutando() const;
    int           tickLimite() const;

    Tarefa* findTarefa(int id);
    CPU*    findCPU(int id);

public:
    ~GerenciadorTarefa();

    static void               configurar(const ConfigSimulacao& config);
    static void               resetar();
    static GerenciadorTarefa* getInstance();

    // Controle da simulação
    void avancar();
    void retroceder();
    void executarCompleto();

    // Edição manual do estado de uma tarefa (invalida história futura)
    void editarEstadoTarefa(int tarefaId, EstadoTarefa novoEstado);

    bool podeAvancar()         const;
    bool podeRetroceder()      const;
    bool isSimulacaoCompleta() const;

    // Leitura de estado
    int                        getTickAtual()  const;
    int                        getQuantum()    const;
    int                        getQtdeCpus()   const;
    const std::vector<CPU>&    getCPUs()       const;
    const std::vector<Tarefa>& getTarefas()    const;
};
