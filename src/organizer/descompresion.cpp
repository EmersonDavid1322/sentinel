#include "descompresion.h"
#include <archive.h>
#include <archive_entry.h>

#include "errores.h"
#include "logger.h"

bool comprobar_archivo_comprimido(const std::string& archivo) {
    struct archive *a = archive_read_new();
    if (!a) {
        throw ErrorOrganizador("No se pudo crear el objeto de lectura de libarchive.");
    }

    struct archive_entry *entry;

    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    archive_read_support_format_raw(a);

    if (archive_read_open_filename(a, archivo.c_str(), 10240) != ARCHIVE_OK) {
        archive_read_free(a);
        throw ErrorOrganizador("No se pudo abrir el archivo: " + archivo);
    }

    // Leer la cabecera
    int r = archive_read_next_header(a, &entry);
    bool es_comprimido = false;

    if (r == ARCHIVE_OK || r == ARCHIVE_WARN) {
        const std::string nombre = archive_filter_name(a, 0);
        logInfo("Archivo comprimido. Filtro detectado: " + nombre, "organizador.log");
        es_comprimido = true;
    }

    archive_read_close(a);
    archive_read_free(a);

    return es_comprimido;
}

void extraer_archivo(const char* ruta_archivo, const std::string& destino) {
    // a=lector ext=extractor o escritor entry=contiene la información del archivo r=Da los resultado de operaciones
    //Inicializara variables
    struct archive* a = archive_read_new();
    struct archive* ext = archive_write_disk_new();
    struct archive_entry* entry;
    int r;

    //Contendra el mensaje de error en dado caso que haya alguno
    std::string error;

    // Configurar banderas predeterminadas para la extracción en disco
    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;

    // Habilitar formatos y filtros
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    // Pasar flags
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);

    // Abrir el archivo comprimido en bloques de 10kb
    if ((r = archive_read_open_filename(a, ruta_archivo, 10240))) {
        error = archive_error_string(a);
        archive_read_free(a);
        archive_write_free(ext);
        throw ErrorOrganizador("Error al abrir el archivo: " + error);
    }

    // Iterar sobre cada elemento/fichero dentro del contenedor comprimido
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        //Construir ruta destino
        std::string archivo_original = archive_entry_pathname(entry);
        std::string ruta_destino = destino + "/" + archivo_original;
        archive_entry_set_pathname(entry, ruta_destino.c_str());

        logInfo("Extrayendo: " + ruta_destino, "organizador.log");

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
                    logError("Error al escribir datos: " + error, "organizador.log");
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