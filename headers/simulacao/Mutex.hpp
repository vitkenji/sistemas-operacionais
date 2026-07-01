#pragma once
#include <deque>
#include <string>

struct Mutex {
    int id = 0;
    std::string donoTarefaID;
    std::deque<std::string> filaEspera;
};
