#include <string>
#include <filesystem>
#include <fstream>
#include <ctime>
#include <mutex>
#include "rutas.h"
namespace fs = std::filesystem;
std::mutex mutex_log;

static void escribirLog(const std::string& nivel, const std::string& mensaje, const std::string& tipo) {
    std::lock_guard<std::mutex> lock(mutex_log);
    fs::path ruta_log = obtenerRutaBase() / "logs" / tipo;

    time_t ahora = time(0);
    std::string fecha = ctime(&ahora);
    std::ofstream log(ruta_log, std::ios::app);
    log << "[" << fecha.substr(0, fecha.size()-1) << "] [" << nivel << "] " << mensaje << std::endl;
}

void logInfo(const std::string& mensaje, const std::string& tipo) {
    escribirLog("INFO", mensaje, tipo);
}

void logWarning(const std::string& mensaje, const std::string& tipo) {
    escribirLog("WARNING", mensaje, tipo);
}

void logError(const std::string& mensaje, const std::string& tipo) {
    escribirLog("ERROR", mensaje, tipo);
}
