#pragma once
#include <string>
#include <fstream>
#include <vector>
#include "config.h"

std::string verificarCarpetas(const std::vector<std::string>& carpetas, const std::string& destino);

void ejecutarBackup(const std::vector<std::string>& carpetas, const std::string& destino);

void registroResultado(const std::string& resultado);

void loopBackup(const ConfigBackup& config);

void hacerBackup(const std::vector<std::string>& carpetas, const std::string& destino, const std::string hora);