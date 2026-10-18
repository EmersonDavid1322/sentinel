#pragma once
#include <string>

bool comprobar_archivo_comprimido(const std::string& archivo);

void extraer_archivo(const char* ruta_archivo, const std::string& destino);