#pragma once
#include <atomic>
#include <mutex>
#include <condition_variable>

extern std::atomic<bool> hayEntornoGrafico;

extern std::atomic<bool> corriendo;
extern std::atomic<bool> corriendo_backup_nube;
extern std::atomic<bool> corriendo_backup_local;
extern std::atomic<bool> modo_test;
extern std::mutex mtx_apagado;
extern std::condition_variable cv_apagado;

void manejoSenal(int senal);
void capturarSenal();

void verficarEntornoGrafico();