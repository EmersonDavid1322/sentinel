#pragma once
#include <string>
#include <thread>
#include <filesystem>
#include <curl/curl.h>
#include "config.h"
#include "errores.h"

size_t escribirRespuesta(void* datos, size_t tamano, size_t cantidad, std::string* salida);

std::string renovarAccessToken(const std::string& refresh_token, const std::string& clienteID, const std::string& clienteSecret);

void actualizarToken(const std::string& token);

void elimarAnteriorBackupNube(const std::string& dirrecion_backup, const std::string& token);

bool verificarSiExisteArchivoDropbox(const std::string& accessToken, const std::string& dropboxPath);

CURL* inicializarCurl(const std::string& contexto);

void accionesPosBackupNube(const ConfigBackupNube &config, const bool &hubo_errores, std::string &token, const std::string &nombre_carpeta);

template <typename Func>
void conReintento(const std::string &refresh_token, const std::string &clienteID, const std::string &clienteSecret,
                  std::string &token, Func operacion) {
    int max_intentos = 3;
    for (int intentos = 1; intentos <= max_intentos; intentos++) {
        try {
            operacion();
            return;
        }
        catch (const ErrorBackupAPI& e) {
            if (e.codigoHTTP == 401 && intentos < max_intentos) {
                token = renovarAccessToken(refresh_token, clienteID, clienteSecret);
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