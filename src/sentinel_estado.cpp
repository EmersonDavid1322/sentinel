#include <atomic>
#include <csignal>
#include "sentinel_estado.h"

std::atomic<bool> corriendo{true};

void manejoSenal(int senal){
    corriendo = false;
}
void capturarSenal(){
    signal(SIGINT, manejoSenal);
    signal(SIGTERM, manejoSenal);
}