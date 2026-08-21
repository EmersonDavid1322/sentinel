#pragma once
#include <string>
#include <thread>
#include <curl/curl.h>
#include "config.h"
#include "errores.h"

size_t escribirRespuesta(void* datos, size_t tamano, size_t cantidad, std::string* salida);

std::string renovarAccessToken(const ConfigBackupNube& config);

void actualizarToken(const std::string& token);

void limpiarLog();

CURL* inicializarCurl(const std::string& contexto);

template <typename Func>
void conReintento(const ConfigBackupNube& config, std::string& token, Func operacion) {
    int max_intentos = 3;
    for (int intentos = 1; intentos <= max_intentos; intentos++) {
        try {
            operacion();
            return;
        }
        catch (const ErrorBackupAPI& e) {
            if (e.codigoHTTP == 401 && intentos < max_intentos) {
                token = renovarAccessToken(config);
                actualizarToken(token);
                continue;
            }
            throw;
        }
        catch (const ErrorBackupRED& e) {
            if (intentos < max_intentos) {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            }
            throw;
        }
    }
    throw DaemonError("Se agotaron los intentos");
}