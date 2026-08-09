#include <atomic>
#include <csignal>
#include <cstdlib>
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

std::atomic<bool> hayEntornoGrafico{false};

void verficarEntornoGrafico() {
    hayEntornoGrafico = std::getenv("DISPLAY") != nullptr || std::getenv("WAYLAND_DISPLAY") != nullptr;
}