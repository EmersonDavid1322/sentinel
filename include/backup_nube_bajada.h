#pragma once
#include "config.h"
#include <filesystem>

size_t escribirEnArchivo(void* datos, size_t tamano, size_t cantidad, std::ofstream* archivo);

void backupNubeBajada(const std::string& token, const std::string& rutaRemota, const std::filesystem::path& rutaLocal);

void ejecutarBajadaArchivosNube(const ConfigBackupNube& config);