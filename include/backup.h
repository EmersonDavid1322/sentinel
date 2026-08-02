#pragma once
#include <string>
#include <vector>
#include "config_compartida.h"

enum class ResultadoVerificacionRecursos {
    OK,
    CANCELADO_DISCO,
    CANCELADO_CPU,
    FORZADO
};

ResultadoVerificacionRecursos verificarRecursosBackup(const ConfigBackup& configBackup, const ConfigMonitor& configMonitor);

std::string verificarCarpetasBackup(const std::vector<std::string>& carpetas, const std::string& destino);

void ejecutarBackup(const std::vector<std::string>& carpetas, const std::string& destino, const std::vector<std::string>& ignorar);

void registroResultado(const std::string& resultado);

void loopBackup(ConfigCompartida& config_compartida);

void hacerBackup(const ConfigBackup& config_backup, const ConfigMonitor& config_monitor);