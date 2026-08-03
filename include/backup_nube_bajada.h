#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include "config_compartida.h"
namespace fs = std::filesystem;

//auxiliar
size_t escribirRespuesta(void* datos, size_t tamano, size_t cantidad, std::string* salida);

bool debeIgnorarce(const fs::path& ruta, const std::vector<std::string>& lista_ignorar);

//subida
void subirArchivo(std::string archivo ,std::string& ruta_remota, std::string& token);

void ejecutarBackupNube(const ConfigBackupNube& config);

void loopBackupNube(ConfigCompartida& config_compartida);