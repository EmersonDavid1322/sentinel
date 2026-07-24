#include <atomic>
#include <csignal>
#include "sentinel_estado.h"

std::atomic<bool> corriendo{true};
std::mutex mtx_apagado;
std::condition_variable cv_apagado;

void manejoSenal(int senal){
    (void)senal;
    corriendo = false;
    cv_apagado.notify_all();
}

void capturarSenal(){
    signal(SIGINT, manejoSenal);
    signal(SIGTERM, manejoSenal);
}