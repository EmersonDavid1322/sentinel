#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <filesystem>
#include <sys/inotify.h>
#include <archive.h>
#include <archive_entry.h>
#include <poll.h>
#include <unistd.h>
#include "organizer.h"
#include "logger.h"
#include "errores.h"
#include "notificador.h"
#include "sentinel_estado.h"
#include "descriptor_inotify.h"
namespace fs = std::filesystem;

std::vector<std::string> verificarCarpetasOrganizado(const std::map<std::string, std::string>& carpetasRegla, const std::string& carpetaVigilar){
    std::vector<std::string> carpetas_fallidas;
    if (!fs::exists(carpetaVigilar)){
        throw ErrorOrganizador("Organizador: Carpeta a vigilar no existente: " + carpetaVigilar);
        enviarNotificación("Organizador", "La carpeta '" + carpetaVigilar + "' no existe", "ERROR");
    }

    for (const auto& par : carpetasRegla){
        if (!fs::exists(par.second)){
            logWarning("Organizador: Carpeta no existende para archivo de extensión: " + par.first + " " + par.second + "\nSe cancelo la acción", "sentinel.log");
            carpetas_fallidas.push_back(par.second);
            enviarNotificación("Organizador",
                                "Organizador: Carpeta no existende para archivo de extensión: '" + par.first + "' '" + par.second + "'", "WARNING");
        }
    }
    return carpetas_fallidas;
}

bool comprobar_archivo_comprimido(const std::string& archivo) {
    struct archive *a = nullptr;
    struct archive_entry *entry;

    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    archive_read_support_format_raw(a);

    if (archive_read_open_filename(a, archivo.c_str(), 10240) != ARCHIVE_OK) {
        throw ErrorOrganizador("No se pudo abrir el archivo: " + archivo);
    }

    int r = archive_read_next_header(a, &entry);
    if (r == ARCHIVE_OK || r == ARCHIVE_WARN) {
        const std::string nombre =  archive_filter_name(a, 0);
        logInfo("Archivo comprimido. Filtro detectado: " + nombre, "organizador.log");
        archive_read_close(a);
        archive_read_free(a);
        return true;
    }
    archive_read_close(a);
    archive_read_free(a);
    return false;
}

void moverArchivo(const std::string &archivo, const std::map<std::string, std::string> &reglas,
                  const std::vector<std::string> &carpetas_fallidas) {
    fs::path ruta(archivo);
    std::string extension = ruta.extension().string();
    try {
        for (const auto& [extension_regla, destino] : reglas) {
            if (extension_regla != extension) continue;

            for (const std::string& carpeta : carpetas_fallidas) {
                if (carpeta == destino) {
                    logWarning("Organizador: saltando archivo, carpeta destino no existe: " + destino, "sentinel.log");
                    return;
                }
            }

            fs::path destino_final = fs::path(destino) / ruta.filename();
            fs::rename(ruta, destino_final);
            logInfo("Organizador: archivo movido: " + archivo + " -> " + destino, "sentinel.log");
            return;
        }
    }
    catch (const fs::filesystem_error& e) {
        logError("Error organizador-" + std::string(e.what()), "sentinel.log");
        enviarNotificación("Organizador", "Error organizador-" + std::string(e.what()), "ERROR");
    }

}

void extraer_archivo(const char* ruta_archivo) {
    // a=lector ext=extractor o escritor entry=contiene la información del archivo r=Da los resultado de operaciones
    struct archive* a;
    struct archive* ext;
    struct archive_entry* entry;
    int r;

    //Contendra el mensaje de error en dado caso que haya alguno
    std::string error;

    // Configurar banderas predeterminadas para la extracción en disco
    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;

    // Inicializar el objeto lector
    a = archive_read_new();
    // Habilitar formatos y filtros
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    // Inicializar el escritor de disco
    ext = archive_write_disk_new();
    // Pasar flags
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);

    // Abrir el archivo comprimido en bloques de 10kb
    if ((r = archive_read_open_filename(a, ruta_archivo, 10240))) {
        error = archive_error_string(a);
        throw ErrorOrganizador("Error al abrir el archivo: " + error);
    }

    // Iterar sobre cada elemento/fichero dentro del contenedor comprimido
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        std::string archivo = archive_entry_pathname(entry);
        logInfo("Extrayendo: " + archivo + "\n", "organizador.log");

        // Escribir la cabecera en el disco
        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK) {
            error = archive_error_string(ext);
            logError("Error en cabecera: " + error, "organizador.log");
        } else if (archive_entry_size(entry) > 0) {
            // datos del elemento hacia el disco de destino
            const void* buff;
            size_t size;
            la_int64_t offset;

            while (true) {
                r = archive_read_data_block(a, &buff, &size, &offset);
                if (r == ARCHIVE_EOF) break;//Finaliza el archivo correctamente
                if (r < ARCHIVE_OK) {
                    error = archive_error_string(a);
                    logError("Error al leer datos: " + error, "organizador.log");
                    break;
                }
                //Continuar extrayendo el archivo
                r = archive_write_data_block(ext, buff, size, offset);
                if (r < ARCHIVE_OK) {
                    error = archive_error_string(ext);
                    logError("Error al leer datos: " + error, "organizador.log");
                    break;
                }
            }
        }
        archive_write_finish_entry(ext);
    }

    // Limpieza y cierre de flujos de memoria
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    logInfo("Archivo Extraido correctamente", "organizador.log");
}

void ejecutarOrganizador(ConfigCompartida& config_compartida){
    try{
        ConfigSentinel config = config_compartida.obtener();
        std::vector<std::string> carpetas_fallidas = verificarCarpetasOrganizado(config.organizador.reglas, config.organizador.carpeta_vigilar);

        auto vigilante = std::make_unique<VigilanteInotify>(config.organizador.carpeta_vigilar, IN_CREATE | IN_MOVED_TO);
        std::string carpetaVigiladaActual = config.organizador.carpeta_vigilar;
        std::map<std::string, std::string> reglas = config.organizador.reglas;

        while (corriendo) {
            config = config_compartida.obtener();

            if (config.organizador.carpeta_vigilar != carpetaVigiladaActual) {
                logInfo("Cambio detectado en carpeta vigilada, recreando vigilante inotify", "sentinel.log");
                carpetas_fallidas = verificarCarpetasOrganizado(config.organizador.reglas, config.organizador.carpeta_vigilar);
                vigilante = std::make_unique<VigilanteInotify>(config.organizador.carpeta_vigilar, IN_CREATE | IN_MOVED_TO);
                carpetaVigiladaActual = config.organizador.carpeta_vigilar;
            }
            if (config.organizador.reglas != reglas) {
                logInfo("Cambio detectado en reglas del organizador, recalculando carpetas fallidas", "sentinel.log");
                reglas = config.organizador.reglas;
                carpetas_fallidas = verificarCarpetasOrganizado(reglas, carpetaVigiladaActual);
            }

            struct pollfd pfd;
            pfd.fd = vigilante->fd;
            pfd.events = POLLIN;

            int resultado = poll(&pfd, 1, 1000);
            
            if (resultado < 0) break;
            
            if (pfd.revents & POLLIN) {
                char buffer[4096];
                int bytes = read(vigilante->fd, buffer, sizeof(buffer));
                if (bytes < 0) break;

                for (int i = 0; i < bytes; ) {
                    struct inotify_event* evento = (struct inotify_event*)&buffer[i];
                    if (evento->len > 0) {
                        std::string rutaCompleta = config.organizador.carpeta_vigilar + "/" + evento->name;
                        moverArchivo(rutaCompleta, config.organizador.reglas, carpetas_fallidas);
                    }
                    i += sizeof(struct inotify_event) + evento->len;
                }
            }
        }
    }
    catch(const ErrorInotify& e){
        logError("Error en Inotify organizador - " + std::string(e.what()), "sentinel.log");
        enviarNotificación("Error en Inotify organizador", "Ocurrio un error en el Inotify organizador: " + std::string(e.what()), "ERROR");
    }
    catch(const ErrorOrganizador& e){
        logError("Error en Organizador - " + std::string(e.what()), "sentinel.log");
        enviarNotificación("Error Organizador", "Ocurrio un error en el organizador: " + std::string(e.what()), "ERROR");
    }
    catch(const DaemonError& e){
        logError("Error en Deamon - " + std::string(e.what()), "sentinel.log");
        enviarNotificación("Error Deamon-Organizador", "Ocurrio un error en el organizador: " + std::string(e.what()), "ERROR");
    }
}
