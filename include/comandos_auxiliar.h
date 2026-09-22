#pragma once
#include "json.hpp"
using json = nlohmann::json;

void enviarRespuesta(const std::string& mensaje);

//json
json leerJSONActual(const std::filesystem::path& ruta);

void guardarJSON(const json& datos, const std::filesystem::path& ruta);

//estado
void cambiarEstadoSeccion(const std::string& seccion, bool activo);

//auxiliar
std::string limpiarEspacios(const std::string& texto);

void cambiarDireccion(const std::string& parametro,const std::string& llave , const std::string& dirrecion);

//ignorar para backup loca y nube
void aniadirIgnorar(const std::string& parametro, const std::string& valor);