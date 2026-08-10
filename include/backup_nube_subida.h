#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include "config_compartida.h"
namespace fs = std::filesystem;

bool debeIgnorarce(const fs::path& ruta, const std::vector<std::string>& lista_ignorar);

//subida
std::string iniciarSesion(const std::string& trozo, const std::string& token);

void continuarSecion(const std::string& sessionId, const size_t& offset, const std::string& trozo, const std::string& token);

void continuarSesion(const std::string& sessionId, const size_t& offset, const std::string& trozo, const std::string& token);

void ejecutarBackupNube(const ConfigBackupNube& config);

void loopBackupNube(ConfigCompartida& config_compartida);