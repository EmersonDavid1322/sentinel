#pragma once
#include <string>
#include "config.h"

double uso_ram();

bool obtener_tics_cpu(long long& trabajo, long long& descanso);
double uso_cpu();

double uso_disco();

void revisarLimites(double& ram, double& cpu, double& disco);
void ejecutarMonitoreo(const int& limite_ram, const int& limite_cpu, const int& limite_disco);
