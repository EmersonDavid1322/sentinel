#pragma once
#include <string>
#include "config_loader.h"
#include "backup.h"

void agregarCarpetaBackup(const std::string& carpeta);

void cambiarForzarBackup(const std::string& accion);

std::string mensajeResultadoBackup(const ResultadoHacerBackup& resultado);