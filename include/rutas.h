#pragma once
#include <filesystem>

void asegurarCarpetasSentinel();

std::filesystem::path obtenerRutaConfig();

std::filesystem::path obtenerRutaLogs();

std::filesystem::path obtenerRutaEstado();