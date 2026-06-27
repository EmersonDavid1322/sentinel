#include <string>
#include <filesystem>
#include <fstream>
#include <ctime>
#include <mutex>
#include "rutas.h"
namespace fs = std::filesystem;
std::mutex mutex_log;

static void escribirLog(const std::string& nivel, const std::string& mensaje) {
    std::lock_guard<std::mutex> lock(mutex_log);
    fs::path ruta_log = obtenerRutaBase() / "logs" / "sentinel.log";
    time_t ahora = time(0);
    std::string fecha = ctime(&ahora);
    std::ofstream log(ruta_log, std::ios::app);
    log << "[" << fecha.substr(0, fecha.size()-1) << "] [" << nivel << "] " << mensaje << std::endl;
}

void logInfo(const std::string& mensaje) {
    escribirLog("INFO", mensaje);
}

void logWarning(const std::string& mensaje) {
    escribirLog("WARNING", mensaje);
}

void logError(const std::string& mensaje) {
    escribirLog("ERROR", mensaje);
}
