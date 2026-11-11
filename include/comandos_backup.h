#pragma once
#include <string>
#include "config_loader.h"
#include "backup.h"

std::string agregarCarpetaBackup(const std::string& carpeta);

std::string cambiarForzarBackup(const std::string& accion);

std::string mensajeResultadoBackup(const ResultadoHacerBackup& resultado);