#pragma once
#include <string>
#include "config.h"

size_t escribirRespuesta(void* datos, size_t tamano, size_t cantidad, std::string* salida);

std::string renovarAccessToken(const ConfigBackupNube& config);
