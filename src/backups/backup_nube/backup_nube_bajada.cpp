#include "backup_nube_bajada.h"
#include "backup_nube_auxiliar_dropbox.h"
#include "backup_nube_listado.h"
#include "backup_nube_auxiliar.h"
#include "auxiliar_compartido.h"
#include "config.h"
#include "errores.h"
#include "logger.h"
#include <vector>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <curl/curl.h>
namespace fs = std::filesystem;

size_t escribirEnArchivo(void* datos, size_t tamano, size_t cantidad, std::ofstream* archivo) {
    size_t bytesTotales = tamano * cantidad;
    archivo->write((char*)datos, bytesTotales);
    return bytesTotales;
}

void backupNubeBajada(const std::string& token, const std::string& rutaRemota, const std::filesystem::path& rutaLocal) {
    CURL* curl = inicializarCurl("Descarga de archivos");

    std::string autorizacion = "Authorization: Bearer " + token;
    std::string header_arg = "Dropbox-API-Arg: {\"path\": \"" + rutaRemota + "\"}";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, autorizacion.c_str());
    headers = curl_slist_append(headers, header_arg.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/download");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::ofstream archivo(rutaLocal);
    if (!archivo.is_open()) {
        curl_easy_cleanup(curl);
        throw ErrorBackup("No se pudo crear el archivo local: " + rutaLocal.string());
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, escribirEnArchivo);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &archivo);

    CURLcode resultado = curl_easy_perform(curl);
    if (resultado != CURLE_OK) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw ErrorBackupRED("Error al subir archivo: " + std::string(curl_easy_strerror(resultado)));
    }

    long codigo_http = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &codigo_http);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    archivo.close();

    if (codigo_http != 200) {
        throw ErrorBackupAPI("Error al subir archivo: ", codigo_http);
    }
    logInfo("Se bajo correctamente al archivo a la ruta: " + rutaLocal.string(), "backups.log");
}

void ejecutarBajadaArchivosNube(const ConfigBackupNube& config) {
    time_t ahora = time(0);
    tm* tiempo = localtime(&ahora);
    char buffer[6];
    strftime(buffer, sizeof(buffer), "%H:%M", tiempo);
    std::string hora_actual = buffer;
    if (hora_actual != config.hora_bajada){
        return;
    }

    logInfo("Se incio la bajada de archivos de la nube: " + hora_actual, "sentinel.log");
    logInfo("Se incio la bajada de archivos de la nube: " + hora_actual + " Destino: " + config.carpeta_destino, "backups.log");
    limpiarLog();

    std::string token = config.token;
    std::vector<ArchivoRemoto> listaNube;

    namespace fs = std::filesystem;

    conReintento(config, token, [&]() {
         listaNube = listaArchivosRemotos(config);
    });

    std::sort(listaNube.begin(), listaNube.end(), [](const ArchivoRemoto& a, const ArchivoRemoto& b) {
    return a.esCarpeta > b.esCarpeta;
    });

    std::string carpetaRemota = config.carpeta_remota;

    for (const auto& archivo : listaNube) {
        fs::path rutaLocal = calcularRutaLocal(archivo.ruta, carpetaRemota, config.carpeta_destino);

        try{
            if (archivo.esCarpeta) {
                fs::create_directories(rutaLocal);
            }else {
                conReintento(config, token, [&]() {
                    backupNubeBajada(token, archivo.ruta, rutaLocal);
                });
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            logError("Ocurrio un error con el manejo de archivos loca: " + std::string(e.what()), "backups.log");
        }
        catch (const ErrorBackupAPI& e) {
            logError("Ocurrio un error con la petición de bajda de archivos: " + std::string(e.what())
            + " ruta remota: " + archivo.ruta + " ruta sistema: " + rutaLocal.string(), "backups.log");
        }
        catch (const ErrorBackupRED& e) {
            logError("Ocurrio un error con la red al intentar bajar un archivo: " + std::string(e.what())
            + " ruta remota: " + archivo.ruta + " ruta sistema: " + rutaLocal.string(), "backups.log");
        }
        catch (const DaemonError& e) {
            logError("Ocurrio un error inesperado: " + std::string(e.what()), "backups.log");
        }
    }
    logInfo("Se a completado la bajada de archivos, revisa 'backups.log para información detallada", "sentinel.log");
    logInfo("Se a completado la bajada de archivos", "backups.log");
}