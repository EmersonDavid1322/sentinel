#include "auxiliar_compartido.h"
#include "errores.h"
#include "rutas.h"
#include <fstream>
#include <filesystem>
#include <string>
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