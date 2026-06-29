#include <iostream>
#include <filesystem>
#include <thread>
#include <mutex>
#include "config_loader.h"
#include "backup.h"
#include "monitor.h"
#include "organizer.h"
#include "logger.h"
#include "rutas.h"
#include "errores.h"
#include "sentinel_config.h"
namespace fs = std::filesystem;

int main() {
    fs::create_directories(obtenerRutaBase() / "logs");
    
    try {
        ConfigSentinel config = cargarConfig("config/sentinel.json");
        logInfo("Sentinel iniciado correctamente 1.0");
        
        std::thread hilo_json(actualizarJSON);
        hilo_json.detach();

        if (config.backup.activo) {
            std::thread hilo_backup(loopBackup, config.backup);
            hilo_backup.detach();
        }

        if (config.monitor.activo) {
            std::thread hilo_monitor(loopMonitor, config.monitor);
            hilo_monitor.detach();
        }

        if (config.organizador.activo) {
            std::thread hilo_organizador(ejecutarOrganizador,
                                        config.organizador.reglas,
                                        config.organizador.carpeta_vigilar);
            hilo_organizador.detach();
        }

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(60));
        }

    } catch (const DaemonError& e) {
        logError("Error critico al iniciar: " + std::string(e.what()));
        return 1;
    }

    return 0;
}