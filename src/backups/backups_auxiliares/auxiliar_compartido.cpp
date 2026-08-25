#include "auxiliar_compartido.h"
#include "errores.h"
#include "rutas.h"
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
namespace fs = std::filesystem;

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