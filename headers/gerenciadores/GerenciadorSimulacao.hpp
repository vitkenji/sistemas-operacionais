#pragma once
#include "config/CarregadorConfig.hpp"
#include "escalonadores/Escalonador.hpp"
#include "simulacao/CPU.hpp"
#include "simulacao/EstadoSistema.hpp"
#include "simulacao/Mutex.hpp"
#include <map>
#include <string>
#include <vector>

// Controla o estado, o avanço e o histórico da simulação de escalonamento.
class GerenciadorSimulacao {
private:
    static GerenciadorSimulacao* instance;

    std::vector<Tarefa>        listaTarefas;
    std::vector<CPU>           cpus;
    std::vector<Mutex>         mutexes;
    Escalonador*               pEscalonador;
    int                        quantum;

    // historico[0] = estado inicial; historico[T] = estado após T ticks
    std::vector<EstadoSistema> historico;
    int                        tickAtual;         // indice no historico
    bool                       simulacaoCompleta;

    explicit GerenciadorSimulacao(const ConfigSimulacao& config);
    Escalonador* criarEscalonador(const std::string& tipo, int alpha);

    void          computarProximoTick();
    void          aplicarEstado(const EstadoSistema& estado);
    EstadoSistema buildSnapshot(const std::vector<std::string>& sorteadas = {},
                                const std::vector<EventoGantt>& eventos = {}) const;
    void          inicializarMutexes();
    bool          processarAcoesMutex(std::vector<EventoGantt>& eventos);
    bool          todasTerminadas() const;
    bool          hasTarefaProntaOuExecutando() const;
    int           tickLimite() const;

    Tarefa* findTarefa(const std::string& id);
    CPU*    findCPU(int id);
    Mutex*  findMutex(int id);
    Mutex&  getOrCreateMutex(int id);

public:
    ~GerenciadorSimulacao();

    static void               configurar(const ConfigSimulacao& config);
    static void               resetar();
    static GerenciadorSimulacao* getInstance();

    // controle da simulação
    void avancar();
    void retroceder();
    void executarCompleto();

    // edicao manual do estado de uma tarefa (invalida história futura)
    void editarEstadoTarefa(const std::string& tarefaId, EstadoTarefa novoEstado);

    bool podeAvancar()         const;
    bool podeRetroceder()      const;
    bool isSimulacaoCompleta() const;

    // leitura de estado
    int                              getTickAtual()   const;
    int                              getQuantum()     const;
    int                              getQtdeCpus()    const;
    const std::vector<CPU>&          getCPUs()        const;
    const std::vector<Tarefa>&       getTarefas()     const;
    // historico[0] = estado inicial; historico[T] = estado após tick T
    const std::vector<EstadoSistema>& getHistorico()  const;
};
