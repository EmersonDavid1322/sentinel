#include "backup_nube_auxiliar.h"
#include <filesystem>
#include "errores.h"
namespace fs = std::filesystem;

std::filesystem::path calcularRutaLocal(const std::string& ruta_archivo, std::string& ruta_remota, const std::string& ruta_destino) {
    if (ruta_remota.front() != '/') {
        ruta_remota = "/" + ruta_remota;
    }

    if (ruta_archivo.size() < ruta_remota.size()) {
        throw ErrorBackup("Ruta remota inválida, más corta que la carpeta remota configurada: '" + ruta_archivo + "'");
    }

    std::string rutaRelativa = ruta_archivo.substr(ruta_remota.size());
    if (!rutaRelativa.empty() && rutaRelativa.front() == '/') {
        rutaRelativa = rutaRelativa.substr(1);
    }
    return fs::path(ruta_destino) / rutaRelativa;
}