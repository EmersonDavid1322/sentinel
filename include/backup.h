#pragma once
#include <string>
#include <vector>
#include "config_compartida.h"
#include "config.h"

std::string verificarCarpetasBackup(const std::vector<std::string>& carpetas, const std::string& destino);

void ejecutarBackup(const std::vector<std::string>& carpetas, const std::string& destino);

void registroResultado(const std::string& resultado);

void loopBackup(ConfigCompartida& config_compartida);

void hacerBackup(const std::vector<std::string>& carpetas, const std::string& destino, const std::string hora);