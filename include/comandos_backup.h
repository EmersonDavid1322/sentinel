#pragma once
#include <string>
#include "config_loader.h"

void agregarCarpetaBackup(const std::string& carpeta);

void cambiarForzarBackup(const std::string& accion);

void ejecutarBackupComando(const ConfigBackup& configBackup, const ConfigMonitor& configMonitor);