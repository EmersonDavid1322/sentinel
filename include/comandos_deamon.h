#pragma once
#include "json.hpp"
using json = nlohmann::json;

json leerJSONActual(const std::filesystem::path& ruta);

void guardarJSON(const json& datos, const std::filesystem::path& ruta);

void cambiarEstadoSeccion(const std::string& seccion, bool activo);

void cambiarValorMonitor(const std::string& parametro, double valor);

void ejecutarCambioValorMonitor(const std::string& parametro, std::string& valor);

void ejecutarMonitoreoComando();