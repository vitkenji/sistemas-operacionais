#pragma once
#include <iostream>

class GerenciadorGrafico
{
private:
    static GerenciadorGrafico* instance;
    GerenciadorGrafico();
public:
    ~GerenciadorGrafico();
    static GerenciadorGrafico* getInstance();


};