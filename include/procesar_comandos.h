#pragma once
#include <string>
#include "config_loader.h"
#include <config_compartida.h>

void enviarRespuesta(const std::string& mensaje);

void procesarEstado(std::string modulo, std::string& accion);

void procesarComandoMonitor(std::string& accion, std::string& valor);

void procesarComandoBackup(std::string& accion, std::string& valor, const ConfigBackup& configBackup);

void procesarComandoOrganizador(std::string& accion, std::string& valor);

void procesarComandoEstado(std::string& accion, const ConfigSentinel& config);

void procesarComando(const std::string& comando, const ConfigSentinel& config);

void loopComandos(ConfigCompartida& config_compartida);