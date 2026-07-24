#pragma once
#include <atomic>
#include <mutex>
#include <condition_variable>

extern std::atomic<bool> corriendo;
extern std::mutex mtx_apagado;
extern std::condition_variable cv_apagado;

void manejoSenal(int senal);
void capturarSenal();