#include "escalonadores/PriopEscalonador.hpp"

PriopEscalonador::PriopEscalonador()
{
}

PriopEscalonador::~PriopEscalonador() = default;

void atualizarTarefas(std::vector<Tarefa>& tarefas, int tempoAtual) {
    std::cout << "[Escalonador Priop] Ajustando tarefas para o tempo " << tempoAtual << "...\n";
    
    // Exemplo de lógica fictícia: 
    // Ele varre as tarefas e toma decisões baseadas no estado anterior
    for (auto& tarefa : tarefas) {
        if (tarefa.getEstado() == EstadoTarefa::Pendente) {
            // Se estava pendente, ele manda executar
            tarefa.mudarEstado(tempoAtual, EstadoTarefa::EmExecucao);
            
        } else if (tarefa.getEstado() == EstadoTarefa::EmExecucao) {
            // Se já estava em execução, no próximo 'tick' de tempo, ela conclui
            tarefa.mudarEstado(tempoAtual, EstadoTarefa::Concluida);
        }
    }
}
