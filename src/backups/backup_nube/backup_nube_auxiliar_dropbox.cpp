#include "backup_nube_auxiliar_dropbox.h"
#include <curl/curl.h>
#include <fstream>
#include <filesystem>
#include "errores.h"
#include <iostream>
#include "json.hpp"
#include "rutas.h"
#include "comandos_auxiliar.h"
namespace fs = std::filesystem;

size_t escribirRespuesta(void* datos, size_t tamano, size_t cantidad, std::string* salida) {
    size_t bytesTotales = tamano * cantidad;
    salida->append((char*)datos, bytesTotales);
    return bytesTotales;
}

std::string renovarAccessToken(const ConfigBackupNube& config) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw DaemonError("Error al intentar inicializar curl para renovar token");
    }

    std::string cuerpo = "grant_type=refresh_token&refresh_token=" + config.refresh_token +
                          "&client_id=" + config.clienteID + "&client_secret=" + config.clienteSecret;

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.dropboxapi.com/oauth2/token");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cuerpo.c_str());

    std::string respuesta;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, escribirRespuesta);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respuesta);

    CURLcode resultado = curl_easy_perform(curl);
    if (resultado != CURLE_OK) {
        curl_easy_cleanup(curl);
        throw ErrorBackupRED("Error al renovar token: " + std::string(curl_easy_strerror(resultado)));
    }

    long codigo_http = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &codigo_http);
    curl_easy_cleanup(curl);

    if (codigo_http != 200) {
        throw ErrorBackupAPI("No se pudo renovar el token: " + respuesta, codigo_http);
    }

    json respuesta_json = json::parse(respuesta);
    return respuesta_json["access_token"];
}

void elimarAnteriorBackupNube(const std::string& dirrecion_backup, const std::string& token) {
    CURL* curl = inicializarCurl("Eliminar anterior backup");

    std::string json_payload = R"({"path": ")" + dirrecion_backup + R"("})";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.dropboxapi.com/2/files/delete_v2");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());

    CURLcode resultado = curl_easy_perform(curl);
    if (resultado != CURLE_OK) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw ErrorBackupRED("Error de red al intentar eliminar el anterior backup: " + std::string(curl_easy_strerror(resultado)));
    }

    long codigo_http = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &codigo_http);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (codigo_http != 200) {
        std::string mensaje_error = "Error de peticion al servidor al intentar eliminar el anterior backup. Codigo HTTP: " + std::to_string(codigo_http);
        throw ErrorBackupAPI("Error de peticion al servidor al intentar eliminar el anterior backup. Codigo HTTP: ", codigo_http);
    }
}

void actualizarToken(const std::string& token) {
    std::filesystem::path rutaConfig = obtenerRutaConfig();

    json datos = leerJSONActual(rutaConfig);

    datos["backup_nube"]["token"] = token;

    guardarJSON(datos, rutaConfig);
}

CURL* inicializarCurl(const std::string& contexto) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw DaemonError("Error al inicializar el curl: " + contexto);
    }
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    return curl;
}