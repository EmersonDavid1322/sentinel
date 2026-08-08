#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include "config_compartida.h"
namespace fs = std::filesystem;

bool debeIgnorarce(const fs::path& ruta, const std::vector<std::string>& lista_ignorar);

//subida
void subirArchivo(const std::string& ruta,const std::string& ruta_remota,const std::string& token);

void ejecutarBackupNube(const ConfigBackupNube& config);

void loopBackupNube(ConfigCompartida& config_compartida);