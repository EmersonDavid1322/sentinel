#include "backup_nube_listado.h"
#include "config.h"
#include "backup_nube_auxiliar.h"
#include "errores.h"
#include <vector>
#include <curl/curl.h>
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
        throw ErrorBackupAPI("Dropbox respondió con código al la peticion del listado de archivos remotos: "
            + std::to_string(codigo_http) + ": " + respuesta, codigo_http);
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

    return hayMas;
}

std::vector<ArchivoRemoto> listaArchivosRemotos(const ConfigBackupNube& config) {
    std::vector<ArchivoRemoto> info_archivos;
    std::string cursorSalida;

    std::string cuerpoInicial = R"({"path": ")" + config.carpeta_remota + R"(", "recursive": true})";

    bool hayMas = obtenerPaginaListado("https://api.dropboxapi.com/2/files/list_folder", cuerpoInicial,
        config.token, info_archivos, cursorSalida);

    while (hayMas) {
        std::string cuerpoContinuar = R"({"cursor": ")" + cursorSalida + R"("})";
        hayMas = obtenerPaginaListado("https://api.dropboxapi.com/2/files/list_folder/continue", cuerpoContinuar,
            config.token, info_archivos, cursorSalida);
    }
    return info_archivos;
}