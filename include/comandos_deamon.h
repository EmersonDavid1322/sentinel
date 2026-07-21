#pragma once
#include "json.hpp"
#include "config_loader.h"
using json = nlohmann::json;

//json
json leerJSONActual(const std::filesystem::path& ruta);

void guardarJSON(const json& datos, const std::filesystem::path& ruta);

//estado
void cambiarEstadoSeccion(const std::string& seccion, bool activo);

//backup
void agregarCarpetaBackup(const std::string& carpeta);

void cambiarDestinoBackup(const std::string& destino);

void ejecutarBackupComando(const ConfigBackup& configBackup);

//monitor
void cambiarValorMonitor(const std::string& parametro, double valor);

void ejecutarCambioValorMonitor(const std::string& parametro, std::string& valor);

void ejecutarMonitoreoComando();