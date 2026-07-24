#pragma once
#include <filesystem>
#include "config_loader.h"
#include "config_compartida.h"

void actualizarJSON(ConfigCompartida& configCompartida);

std::ifstream comprobar_json(const std::filesystem::path& ruta);

void asegurarConfigExiste(const std::filesystem::path& rutaJSON); 
void crearConfigPorDefecto(const std::filesystem::path& rutaJSON);