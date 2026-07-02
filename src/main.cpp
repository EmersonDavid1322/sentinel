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
        logInfo("Sentinel iniciado correctamente 1.2");
        
        std::thread hilo_json(actualizarJSON);

        std::thread hilo_backup;
        if (config.backup.activo) {
            hilo_backup = std::thread(loopBackup, config.backup);
        }

        std::thread hilo_monitor;
        if (config.monitor.activo) {
            hilo_monitor = std::thread(loopMonitor, config.monitor);
        }

        std::thread hilo_organizador;
        if (config.organizador.activo) {
            hilo_organizador = std::thread(ejecutarOrganizador, 
                                        config.organizador.reglas, 
                                        config.organizador.carpeta_vigilar);
        }

        hilo_json.join();
        if (hilo_backup.joinable()) hilo_backup.join();
        if (hilo_monitor.joinable()) hilo_monitor.join();
        if (hilo_organizador.joinable()) hilo_organizador.join();

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