#include "backup_nube_subida.h"
#include "errores.h"
#include "sentinel_estado.h"
#include "logger.h"
#include "backup_auxiliar.h"
#include "backup_nube_auxiliar.h"
#include "backup_nube_bajada.h"
#include <string>
#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "json.hpp"
using json = nlohmann::json;

std::string iniciarSesion(const std::string& trozo, const std::string& token) {
    CURL* curl = inicializarCurl("Iniciar seción subida");

    std::string autorizacion = "Authorization: Bearer " + token;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, autorizacion.c_str());
    headers = curl_slist_append(headers, "Dropbox-API-Arg: {\"close\": false}");
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

    curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/upload_session/start");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, trozo.data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, trozo.size());

    std::string respuesta;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, escribirRespuesta);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respuesta);

    CURLcode resultado = curl_easy_perform(curl);
    if (resultado != CURLE_OK) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw ErrorBackupRED("Error al iniciar sesión: " + std::string(curl_easy_strerror(resultado)));
    }

    long codigo_http = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &codigo_http);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (codigo_http != 200) {
        throw ErrorBackupAPI("Error al iniciar sesión: " + respuesta, codigo_http);
    }

    json respuesta_json = json::parse(respuesta);
    return respuesta_json["session_id"];
}

void continuarSesion(const std::string& sessionId, const size_t& offset, const std::string& trozo, const std::string& token) {
    CURL* curl = inicializarCurl("Continuar sección de subida");

    std::string autorizacion = "Authorization: Bearer " + token;

    std::string argumento = "{\"cursor\": {\"session_id\": \"" + sessionId + "\", \"offset\": " + std::to_string(offset) + "}, \"close\": false}";
    std::string header_arg = "Dropbox-API-Arg: " + argumento;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, autorizacion.c_str());
    headers = curl_slist_append(headers, header_arg.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

    curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/upload_session/append_v2");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, trozo.data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, trozo.size());

    std::string respuesta;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, escribirRespuesta);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respuesta);

    CURLcode resultado = curl_easy_perform(curl);
    if (resultado != CURLE_OK) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw ErrorBackupRED("Error al continuar la sesión: " + std::string(curl_easy_strerror(resultado)));
    }

    long codigo_http = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &codigo_http);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (codigo_http != 200) {
        throw ErrorBackupAPI("Error al continuar la sesión: " + respuesta, codigo_http);
    }
}

void finalizarSesion(const std::string& sessionId, const size_t& offset, const std::string& dirrecion,
    const std::string& trozo, const std::string& token) {
    CURL* curl = inicializarCurl("Finzalizar seción subida");

    std::string autorizacion = "Authorization: Bearer " + token;

    json arg;
    arg["cursor"]["session_id"] = sessionId;
    arg["cursor"]["offset"] = offset;

    arg["commit"]["path"] =dirrecion;
    arg["commit"]["mode"] = "overwrite";
    std::string header_arg = "Dropbox-API-Arg: " + arg.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, autorizacion.c_str());
    headers = curl_slist_append(headers, header_arg.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

    curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/upload_session/finish");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, trozo.data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, trozo.size());

    std::string respuesta;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, escribirRespuesta);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respuesta);

    CURLcode resultado = curl_easy_perform(curl);
    if (resultado != CURLE_OK) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw ErrorBackupRED("Error al finalizar la sesión: " + std::string(curl_easy_strerror(resultado)));
    }

    long codigo_http = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &codigo_http);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (codigo_http != 200) {
        throw ErrorBackupAPI("Error al finalizar la sesión: " + respuesta, codigo_http);
    }
}

void subirArchivoStreaming(const std::string& ruta, const std::string& rutaRemota, const std::string& token) {
    std::ifstream archivo(ruta, std::ios::binary | std::ios::ate);
    if (!archivo.is_open()) {
        throw ErrorBackup("No se pudo abrir el archivo: " + ruta);
    }

    std::streamsize tamañoTotal = archivo.tellg();
    archivo.seekg(0, std::ios::beg);

    const size_t TAMANO_TROZO = 8 * 1024 * 1024;
    size_t offset = 0;
    bool esPrimero = true;
    std::string sessionId;

    while (static_cast<std::streamsize>(offset) < tamañoTotal) {
        size_t restante = tamañoTotal - offset;
        size_t tamañoLectura = std::min(restante, TAMANO_TROZO);
        bool esUltimo = (restante <= TAMANO_TROZO);

        std::string trozo(tamañoLectura, '\0');
        archivo.read(&trozo[0], tamañoLectura);

        if (esPrimero) {
            sessionId = iniciarSesion(trozo, token);
            esPrimero = false;

            if (esUltimo) {
                finalizarSesion(sessionId, tamañoLectura, rutaRemota, "", token);
                logInfo("Se a subido correctamente el archivo " + rutaRemota, "backups.log");
            }
        } else if (esUltimo) {
            finalizarSesion(sessionId, offset, rutaRemota, trozo, token);
            logInfo("Se a subido correctamente el archivo " + rutaRemota, "backups.log");
        } else {
            continuarSesion(sessionId, offset, trozo, token);
        }
        offset += tamañoLectura;
    }
}

void ejecutarBackupNube(const ConfigBackupNube& config) {
    time_t ahora = time(0);
    tm* tiempo = localtime(&ahora);
    char buffer[6];
    strftime(buffer, sizeof(buffer), "%H:%M", tiempo);
    std::string hora_actual = buffer;
    if (hora_actual != config.hora){
        return;
    }

    logInfo("Se incio el backup a la nube " + hora_actual, "sentinel.log");
    logInfo("Se incio el backup a la nube " + hora_actual + " Destino: " + config.carpeta_remota, "backups.log");
    limpiarLog();
    std::string token = config.token;
    bool hubo_errores = false;

    namespace fs = std::filesystem;
    for (const auto& carpeta : config.carpetas) {
        fs::path origen(carpeta);
        for (auto it = fs::recursive_directory_iterator(origen); it != fs::recursive_directory_iterator(); ++it) {
            const auto& entrada = *it;

            fs::path archivo = origen / entrada;

            if (fs::is_directory(entrada) && debeIgnorarce(entrada.path(), config.ignorar)) {
                it.disable_recursion_pending();
                logWarning("Se ignoro la carpeta completa: " + entrada.path().string(), "backups.log");
                continue;
            }

            if (debeIgnorarce(entrada.path(), config.ignorar)) {
                logWarning("Se ignoro un archivo: " + entrada.path().string(), "backups.log");
                continue;
            }

            if (!fs::is_regular_file(entrada)) {
                logWarning("Se ignoro un archivo de tipo no regular: " + archivo.string(), "backups.log");
                continue;
            }

            fs::path ruta_relativa = fs::relative(entrada.path(), origen);
            std::string ruta_remota = config.carpeta_remota + "/" + ruta_relativa.string();

            try{
                subirArchivoStreaming(archivo.string(), ruta_remota, token);
            }
            catch (const std::filesystem::filesystem_error& e) {
                logError("Ocurrio un error con el manejo de archivos loca: " + std::string(e.what()), "backups.log");
                hubo_errores = true;
            }
            catch (const ErrorBackupAPI& e) {
                if (e.codigoHTTP == 401) {
                    token = renovarAccessToken(config);
                    actualizarToken(token);
                    logInfo("Se a actualizado el token", "backups.log");
                    subirArchivoStreaming(archivo.string(),ruta_remota, token);
                }else {
                    logError("Ocurrio un error con la petición del backup: " + std::string(e.what())
                    + " ruta remota: " + ruta_remota + " ruta sistema: " + archivo.string(), "backups.log");
                    hubo_errores = true;
                }
            }
            catch (const ErrorBackupRED& e) {
                logError("Ocurrio un error con la red al intentar realizar el backup a la nube" + std::string(e.what())
                + " ruta remota: " + ruta_remota + " ruta sistema: " + archivo.string(), "backups.log");
                hubo_errores = true;
            }
            catch (const DaemonError& e) {
                logError("Ocurrio un error inesperado: " + std::string(e.what()), "backups.log");
                hubo_errores = true;
            }
        }
    }
    if (!hubo_errores) {
        logInfo("Se compelto el backup a DropBox de forma correcta", "sentinel.log");
        logInfo("Se compelto el backup a DropBox de forma correcta", "backups.log");
    }else {
        logInfo("Se completo el backup a DropBox, hubo problemas con algunos archivos, por favor revise 'backups.log' para mas información", "sentinel.log");
        logInfo("Se completo el backup a DropBox, hubo algunos error con archivos", "backups.log");
    }
}

void loopBackupNube(ConfigCompartida& config_compartida) {
    while (corriendo) {
        ConfigSentinel config = config_compartida.obtener();

        if (config.backup_nube.activo) {
            ejecutarBackupNube(config.backup_nube);
            ejecutarBajadaArchivosNube(config.backup_nube);
        }
        std::unique_lock<std::mutex> lock(mtx_apagado);
        cv_apagado.wait_for(lock, std::chrono::seconds(60), [] { return !corriendo.load(); });
    }
}