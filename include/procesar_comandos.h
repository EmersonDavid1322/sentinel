#pragma once
#include <string>
#include <config_compartida.h>

void enviarRespuesta(const std::string& mensaje);

void procesarEstado(std::string modulo, const std::string& accion);

void procesarComandoMonitor(std::string& accion, std::string& valor);

void procesarComandoBackup(const std::string& accion, const std::string& valor, const ConfigBackup& configBackup, const ConfigMonitor& configMonitor);

void procesarComandoBN(const std::string& accion, const std::string& valor, const ConfigBackupNube& config);

void procesarComandoOrganizador(const std::string& accion, const std::string& valor);

void procesarComandoEstado(const std::string& accion, const ConfigSentinel& config);

void procesarComando(const std::string& comando, const ConfigSentinel& config);

void loopComandos(ConfigCompartida& config_compartida);