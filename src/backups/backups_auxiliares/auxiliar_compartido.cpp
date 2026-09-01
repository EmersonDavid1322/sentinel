#include "auxiliar_compartido.h"
#include "errores.h"
#include "rutas.h"
#include <fstream>
#include <filesystem>
#include <string>
#include <sys/stat.h>
#include <ctime>
#include <vector>
namespace fs = std::filesystem;

bool verificarHoraBackup(const std::string& horaConfigurada) {
    time_t ahora = time(0);
    tm* tiempo = localtime(&ahora);
    char buffer[6];
    strftime(buffer, sizeof(buffer), "%H:%M", tiempo);
    std::string hora_actual = buffer;
    if (hora_actual != horaConfigurada){
        return false;
    }
    return true;
}

bool debeIgnorarce(const fs::path& ruta, const std::vector<std::string>& lista_ignorar) {
    for (const auto& parte : ruta) {
        for (const auto& regla : lista_ignorar) {
            if (parte == regla || parte.extension() == regla) {
                return true;
            }
        }
    }
    return false;
}

void limpiarLog() {
    namespace fs = std::filesystem;
    fs::path logPath = obtenerRutaLogs() / "backups.log";
    std::ofstream logFile(logPath, std::ios::trunc);

    if (!logFile.is_open()) {
        throw ErrorBackup("No se podido abrir el archivo: " + logPath.string());
    }
    logFile.close();
}

bool archivoModificadoCreadoHoy(const fs::path& ruta) {
    if (!fs::exists(ruta)) {
        throw ErrorBackup("Hubo un error con la ruta: " + ruta.string() + " ruta no existente");
    }

    struct stat atributos;

    if (stat(ruta.c_str(), &atributos) != 0) {
        throw ErrorBackup("Ocurrio un erro al intentar leer la fecha del archivo en la ruta: " + ruta.string());
    }

    std::time_t tiempo_archivo = atributos.st_mtime;
    std::time_t tiempo_actual = std::time(nullptr);

    std::tm tm_archivo = *std::localtime(&tiempo_archivo);
    std::tm tm_actual = *std::localtime(&tiempo_actual);

    return (tm_archivo.tm_mday == tm_actual.tm_mday &&
            tm_archivo.tm_mon  == tm_actual.tm_mon  &&
            tm_archivo.tm_year == tm_actual.tm_year);
}

std::string obtenerNombreCarpetaBackup() {
    std::time_t tiempo_actual = std::time(nullptr);
    std::tm tm_actual = *std::localtime(&tiempo_actual);

    char buffer[80];

    std::strftime(buffer, sizeof(buffer), "backup_%Y_%m_%d", &tm_actual);

    return std::string(buffer);
}