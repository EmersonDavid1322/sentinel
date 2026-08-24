#pragma once
#include <filesystem>
#include <string>

std::filesystem::path calcularRutaLocal(const std::string& ruta_archivo, std::string& ruta_remota, const std::string& ruta_destino);