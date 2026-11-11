#pragma once
#include <string>
#include <config_compartida.h>

std::string procesarEstado(std::string modulo, const std::string& accion);

std::string procesarComandoMonitor(std::string& accion, std::string& valor);

std::string procesarComandoBackup(const std::string& accion, const std::string& valor, const ConfigBackup& configBackup, const ConfigMonitor& configMonitor);

std::string procesarComandoBackupNube(const std::string& accion, const std::string& valor, const ConfigBackupNube& config);

std::string procesarComandoOrganizador(const std::string& accion, const std::string& valor);

std::string procesarComandoEstado(const std::string& accion, const ConfigSentinel& config);

std::string procesarComando(const std::string& comando, const ConfigSentinel& config);

void loopComandos(ConfigCompartida& config_compartida);