#include "backup_nube.h"
#include "errores.h"
#include "sentinel_estado.h"
#include "logger.h"
#include  <string>
#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <iostream>

size_t escribirRespuesta(void* datos, size_t tamano, size_t cantidad, std::string* salida) {
    size_t bytesTotales = tamano * cantidad;
    salida->append((char*)datos, bytesTotales);
    return bytesTotales;
}

bool debeIgnorarce(const fs::path& ruta, const std::vector<std::string>& lista_ignorar) {
    for (const auto& parte : ruta) {
        for (const auto& regla : lista_ignorar) {
            if (parte == regla) {
                return true;
            }
        }
    }
    return false;
}

void subirArchivo(std::string ruta, std::string& ruta_remota, std::string token) {
    std::ifstream archivo(ruta, std::ios::binary | std::ios::ate);
    if (!archivo.is_open()) {
        throw ErrorBackup("Error en el archivo: " + ruta);
    }

    std::streamsize tamaño = archivo.tellg();
    archivo.seekg(0, std::ios::beg);
    std::string datosArchivo(tamaño, '\0');

    archivo.read(&datosArchivo[0], tamaño);

    CURL* curl = curl_easy_init();
    if (!curl) {
        curl_easy_cleanup(curl);
        throw DaemonError("Error al intentar incializar el curl de dropbox");
    }

    std::string autorizacion = "Authorization: Bearer " + token;
    std::string header_arg = "Dropbox-API-Arg: {\"path\": \"" + ruta_remota + "\", \"mode\": \"overwrite\"}";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, autorizacion.c_str());
    headers = curl_slist_append(headers, header_arg.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

    curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/upload");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, datosArchivo.data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(tamaño));

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

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    logInfo("Se subio el archvio\n" + respuesta, "backups.log");
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

    namespace fs = std::filesystem;
    try {
        for (std::string carpeta : config.carpetas) {
            fs::path origen(carpeta);
            for (const auto& entrada : fs::recursive_directory_iterator(origen)) {
                fs::path archivo = origen / entrada;

                if (!fs::is_regular_file(entrada)) {
                    logWarning("Se ignoro un archivo de tipo no regular: " + archivo.string(), "backups.log");
                    continue;
                }

                if (debeIgnorarce(entrada.path(), config.ignorar)) {
                    logWarning("Se ignoro un archivo :" + entrada.path().string(), "backups.log");
                    continue;
                }

                fs::path ruta_relativa = fs::relative(entrada.path(), origen);
                std::string ruta_remota = config.carpeta_remota + "/" + ruta_relativa.string();

                subirArchivo(archivo.string(),ruta_remota, config.token);
            }
            logInfo("Se a completado el backup de forma exitosa", "backups.log");
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        logError("Ocurrio un error con el manejo de archivos loca: " + std::string(e.what()), "sentinel.log");
    }
    catch (const ErrorBackupRED& e) {
        logError("Ocurrio un error con la red al intentar realizar el backup a la nube" + std::string(e.what()), "sentinel.log");
    }
    catch (const ErrorBackupAPI& e) {
        logError("Ocurrio un error con la petición del backup: " + std::string(e.what()), "sentinel.log");
    }
    catch (const DaemonError& e) {
        logError("Ocurrio un error inesperado: " + std::string(e.what()), "sentinel.log");
    }
}

void loopBackupNube(ConfigCompartida& config_compartida) {
    while (corriendo) {
        ConfigSentinel config = config_compartida.obtener();

        if (config.backup_nube.activo) {
            ejecutarBackupNube(config.backup_nube);
        }

        std::unique_lock<std::mutex> lock(mtx_apagado);
        cv_apagado.wait_for(lock, std::chrono::seconds(60), [] { return !corriendo.load(); });
    }
}
