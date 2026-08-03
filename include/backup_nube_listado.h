#pragma once
#include "config.h"

bool obtenerPaginaListado(const std::string& url, const std::string& cuerpo,
const std::string& token, std::vector<ArchivoRemoto>& lista,
std::string& cursorSalida);