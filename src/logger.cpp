#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <ctime>
#include <mutex>
#include "rutas.h"
#include "sentinel_estado.h"
namespace fs = std::filesystem;
std::mutex mutex_log;

static void escribirLog(const std::string& nivel, const std::string& mensaje, const std::string& tipo) {
    if (modo_test) {
        std::cout << mensaje << std::endl;
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_log);
    fs::path ruta_log = obtenerRutaLogs() / tipo;

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
