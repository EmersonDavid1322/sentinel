#include <iostream>
#include <filesystem>
#include "config_loader.h"
#include "backup.h"
#include "monitor.h"
#include "organizer.h"
#include "logger.h"
#include "rutas.h"
#include "errores.h"

namespace fs = std::filesystem;

int main() {
    fs::create_directories(obtenerRutaBase() / "logs");
    
    try {
        ConfigSentinel config = cargarConfig("config/sentinel.json");
        logInfo("Sentinel iniciado correctamente");
        
    } catch (const DaemonError& e) {
        logError("Error critico al iniciar: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}