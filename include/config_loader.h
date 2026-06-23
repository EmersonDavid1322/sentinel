#pragma once
#include "json.hpp"
#include <string>
#include <fstream>
#include "config.h"

std::ifstream comprobar_json(const nlohmann::json& datos);

ConfigBackup cargarBackup(const nlohmann::json& datos);

ConfigMonitor cargarMonitor(const nlohmann::json& datos);

ConfigOrganizador cargarOrganizador(const nlohmann::json& datos);

ConfigSentinel cargarConfig(const std::string& rutaJSON);
