#pragma once
#include "json.hpp"
#include "config_loader.h"
using json = nlohmann::json;

//json
json leerJSONActual(const std::filesystem::path& ruta);

void guardarJSON(const json& datos, const std::filesystem::path& ruta);

//estado
void cambiarEstadoSeccion(const std::string& seccion, bool activo);

//auxiliar
std::string limpiarEspacios(const std::string& texto);

void cambiarDireccion(const std::string& parametro,const std::string& llave , const std::string& dirrecion);