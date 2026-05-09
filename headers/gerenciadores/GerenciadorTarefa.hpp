#pragma once
// GerenciadorTarefa.hpp
// Singleton que orquestra toda a simulação: relógio global, motor tick-a-tick,
// histórico para undo/redo e interface de controle para a UI (req. 1 e 3.4).
//
// Decisões de design:
// - Singleton: há exatamente uma simulação ativa por vez; permite acesso direto
//   de qualquer componente de UI sem passar referências pela hierarquia de telas.
// - Histórico por snapshot (EstadoSistema): cada tick gera um snapshot completo
//   dos estados mutáveis (não dos parâmetros fixos). Isso torna avancar() e
//   retroceder() O(1) — basta indexar historico[tickAtual]. (req. 1.5.2)
// - Escalonador por Strategy: criarEscalonador() instancia o algoritmo correto;
//   o motor chama apenas a interface Escalonador::escalonar() — sem if/switch
//   no loop de simulação. (req. 4.2)

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

    // historico[0] = estado inicial (nenhum tick executado)
    // historico[T] = estado do sistema após o tick T
    // Permite navegar avancar/retroceder em O(1) por tick. (req. 1.5.2)
    std::vector<EstadoSistema> historico;
    int                        tickAtual;          // índice corrente no historico
    bool                       simulacaoCompleta;  // true quando todas as tarefas terminaram

    explicit GerenciadorTarefa(const ConfigSimulacao& config);

    // Instancia o escalonador correspondente à string (ex.: "srtf", "priop").
    // Ponto único de extensão para novos algoritmos (req. 4.2).
    Escalonador* criarEscalonador(const std::string& tipo);

    // ── Motor de simulação ────────────────────────────────────────────────────
    // Avança um tick: trata finalizações, preempções de quantum, chegadas,
    // chama o escalonador e registra o novo snapshot no historico.
    void          computarProximoTick();

    // Restaura o estado do sistema (tarefas e CPUs) a partir de um snapshot.
    // Usado por avancar() (redo) e retroceder() (undo). (req. 1.5.2)
    void          aplicarEstado(const EstadoSistema& estado);

    // Cria um snapshot do estado atual para armazenar no historico.
    EstadoSistema buildSnapshot(const std::vector<int>& sorteadas = {}) const;

    // Retorna true quando todas as tarefas estão no estado Terminada.
    bool          todasTerminadas() const;

    // Retorna true quando há ao menos uma tarefa Pronta ou em Execucao.
    // Usado para decidir se CPUs devem ficar ligadas ou desligar (req. 1.2).
    // Nota: não inclui estado Nova — tarefas que ainda não chegaram não podem
    // ser executadas e não devem impedir o desligamento das CPUs.
    bool          hasTarefaProntaOuExecutando() const;

    // Limite de segurança para o loop de executarCompleto() (evita loop infinito).
    int           tickLimite() const;

    Tarefa* findTarefa(int id);
    CPU*    findCPU(int id);

public:
    ~GerenciadorTarefa();

    // Cria (ou recria) a instância com a configuração fornecida.
    static void               configurar(const ConfigSimulacao& config);
    // Destrói a instância atual e libera recursos.
    static void               resetar();
    // Retorna o ponteiro para a instância (nullptr se resetar() foi chamado antes).
    static GerenciadorTarefa* getInstance();

    // ── Controle da simulação (req. 1.5) ─────────────────────────────────────
    void avancar();          // avança um tick (ou refaz se já calculado)
    void retroceder();       // retrocede um tick (undo)
    void executarCompleto(); // avança até o fim sem intervenção do usuário

    // Modifica o estado de uma tarefa no tick atual e invalida o futuro calculado.
    // Se novoEstado == Execucao, atribui a tarefa a uma CPU livre (se houver);
    // caso não haja CPU disponível, a operação é ignorada. (req. 3.4)
    void editarEstadoTarefa(int tarefaId, EstadoTarefa novoEstado);

    bool podeAvancar()         const;
    bool podeRetroceder()      const;
    bool isSimulacaoCompleta() const;

    // ── Leitura de estado (usada pela UI) ─────────────────────────────────────
    int                               getTickAtual()  const;
    int                               getQuantum()    const;
    int                               getQtdeCpus()   const;
    const std::vector<CPU>&           getCPUs()       const;
    const std::vector<Tarefa>&        getTarefas()    const;
    const std::vector<EstadoSistema>& getHistorico()  const;
};
