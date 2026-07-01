#pragma once
#include <atomic>

extern std::atomic<bool> corriendo;

void manejoSenal(int senal);
void capturarSenal();