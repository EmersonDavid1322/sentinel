#pragma once
#include <string>
#include <filesystem>
#include "config_loader.h"

void actualizarJSON();

std::ifstream comprobar_json(const std::filesystem::path& ruta);

void asegurarConfigExiste(const std::filesystem::path& rutaJSON); 
void crearConfigPorDefecto(const std::filesystem::path& rutaJSON);