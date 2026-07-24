#pragma once
#include "config.h"
#include "config_compartida.h"

double uso_ram();

bool obtener_tics_cpu(long long& trabajo, long long& descanso);
double uso_cpu();

double uso_disco();

void revisarLimites(const ConfigMonitor& config , const MetricasSistema& metricas);
void ejecutarMonitoreo(const ConfigMonitor& config);

void loopMonitor(ConfigCompartida& config_compartida);