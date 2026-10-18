#pragma once
#include <map>
#include <string>
#include <vector>
#include "config_compartida.h"

std::vector<std::string> verificarCarpetasOrganizado(const std::map<std::string, std::string>& carpetasRegla, const std::string& carpetaVigilar);

void moverArchivo(const std::string& archivo, const std::map<std::string, std::string>& reglas, const std::vector<std::string>& carpetas_fallidas);

void ejecutarOrganizador(ConfigCompartida& config_compartida);