#include "backup_nube_listado.h"
#include "config.h"
#include "backup_nube_bajada.h"
#include "errores.h"
#include <vector>
#include <curl/curl.h>
#include <iostream>

#include "comandos_auxiliar.h"

bool obtenerPaginaListado(const std::string& url, const std::string& cuerpo,
                          const std::string& token, std::vector<ArchivoRemoto>& lista,
                          std::string& cursorSalida) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw DaemonError("Error al intentar incializar el curl de dropbox");
    }

    std::string autorizacion = "Authorization: Bearer " + token;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, autorizacion.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cuerpo.c_str());

    std::string respuesta;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, escribirRespuesta);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respuesta);

    CURLcode resultado = curl_easy_perform(curl);

    if (resultado != CURLE_OK) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw ErrorBackupRED("Error en la petición: " + std::string(curl_easy_strerror(resultado)));
    }

    long codigo_http = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &codigo_http);

    if (codigo_http != 200) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw ErrorBackupAPI("Dropbox respondió con código " + std::to_string(codigo_http) + ": " + respuesta);
    }

    json respuesta_jso = json::parse(respuesta);

    for (const auto& entrada : respuesta_jso["entries"]) {
        ArchivoRemoto archvio;
        archvio.nombre = entrada["name"];
        archvio.ruta = entrada["path_display"];
        if (entrada[".tag"] == "folder") {
            archvio.esCarpeta = true;
        }else {
            archvio.esCarpeta = false;
        }
        lista.push_back(archvio);
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    cursorSalida = respuesta_jso["cursor"];
    bool hayMas = respuesta_jso["has_more"];
    if (hayMas) {
        return true;
    }
    return false;
}
