#include "gerenciadores/GerenciadorTarefa.hpp"

GerenciadorTarefa* GerenciadorTarefa::instance = nullptr;

GerenciadorTarefa* GerenciadorTarefa::getInstance()
{
    if(instance == nullptr)
    {
        instance = new GerenciadorTarefa();
    }
    return instance;
}

GerenciadorTarefa::GerenciadorTarefa()
{
    
}
