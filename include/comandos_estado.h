#pragma once
#include "config_loader.h"

std::string  estadoBackup(const ConfigBackup& config);

std::string  estadoMonitor(const ConfigMonitor& config);

std::string  estadoOrganizador(const ConfigOrganizador& config);

std::string generarDiagnostico(const ConfigSentinel& config);
