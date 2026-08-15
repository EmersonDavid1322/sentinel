#pragma once
#include <string>
#include <curl/curl.h>
#include "config.h"

size_t escribirRespuesta(void* datos, size_t tamano, size_t cantidad, std::string* salida);

std::string renovarAccessToken(const ConfigBackupNube& config);

void actualizarToken(const std::string& token);

void limpiarLog();

CURL* inicializarCurl(const std::string& contexto);