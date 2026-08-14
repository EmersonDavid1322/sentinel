#pragma once
#include <filesystem>

std::filesystem::path obtenerRutaBase();

void asegurarCarpetasSentinel();

std::filesystem::path obtenerRutaConfig();

std::filesystem::path obtenerRutaLogs();

std::filesystem::path obtenerRutaEstado();