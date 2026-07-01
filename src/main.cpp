#include <iostream>
#include <filesystem>
#include <thread>
#include <mutex>
#include <libnotify/notify.h>
#include "config_loader.h"
#include "backup.h"
#include "monitor.h"
#include "organizer.h"
#include "logger.h"
#include "rutas.h"
#include "errores.h"
#include "sentinel_config.h"
#include "sentinel_estado.h"
namespace fs = std::filesystem;


int main() {
    notify_init("Sentinel");
    capturarSenal();
    fs::create_directories(obtenerRutaBase() / "logs");
    
    try {
        ConfigSentinel config = cargarConfig("config/sentinel.json");
        logInfo("Sentinel iniciado correctamente 1.1");
        
        std::thread hilo_json(actualizarJSON);
        hilo_json.join();

        if (config.backup.activo) {
            std::thread hilo_backup(loopBackup, config.backup);
            hilo_backup.join();
        }

        if (config.monitor.activo) {
            std::thread hilo_monitor(loopMonitor, config.monitor);
            hilo_monitor.join();
        }

        if (config.organizador.activo) {
            std::thread hilo_organizador(ejecutarOrganizador, config.organizador.reglas, config.organizador.carpeta_vigilar);
            hilo_organizador.join();
        }

    } catch (const DaemonError& e) {
        logError("Error critico al iniciar: " + std::string(e.what()));
        notify_uninit();
        logInfo("Sentinel detenido correctamente");
        return 1;
    }    
    notify_uninit();
    logInfo("Sentinel detenido correctamente");
    return 0;
}