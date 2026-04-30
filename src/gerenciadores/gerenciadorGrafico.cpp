#include "gerenciadores/gerenciadorGrafico.hpp"

GerenciadorGrafico* GerenciadorGrafico::instance = nullptr;

GerenciadorGrafico* GerenciadorGrafico::getInstance()
{
    if(instance == nullptr)
    {
        instance = new GerenciadorGrafico();
    }
    return instance;
}

GerenciadorGrafico::GerenciadorGrafico()
{
    
}