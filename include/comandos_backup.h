#pragma once
#include <string>
#include "config_loader.h"

void agregarCarpetaBackup(const std::string& carpeta);

void ejecutarBackupComando(const ConfigBackup& configBackup);