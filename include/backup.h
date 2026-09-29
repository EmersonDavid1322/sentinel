#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include "config_compartida.h"

enum class ResultadoVerificacionRecursos {
    OK,
    CANCELADO_DISCO,
    CANCELADO_CPU,
    FORZADO
};

struct ResultadoCopiaBackup {
    bool completado;
    std::filesystem::path ruta_backup;
};

ResultadoVerificacionRecursos verificarRecursosBackup(const ConfigBackup& configBackup, const ConfigMonitor& configMonitor);

void validarConfiguracionBackup(const ConfigBackup& configBackup);

ResultadoCopiaBackup copiarCarpetasBackup(const ConfigBackup& configBackup);

bool ejecutarBackup(const ConfigBackup& configBackup);

void registroResultado(const std::string& resultado);

void loopBackup(ConfigCompartida& config_compartida);

void hacerBackup(const ConfigBackup& config_backup, const ConfigMonitor& config_monitor);
