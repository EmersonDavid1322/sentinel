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
#include "procesar_comandos.h"
#include "config_compartida.h"
namespace fs = std::filesystem;


int main() {
    notify_init("Sentinel");
    capturarSenal();
    fs::create_directories(obtenerRutaBase() / "logs");
    
    std::filesystem::path rutaConfig = obtenerRutaBase() / "config" / "sentinel.json";

    try {
        asegurarConfigExiste(rutaConfig);
        ConfigCompartida config_compartida;
        config_compartida.actualizar(cargarConfig(rutaConfig));
        logInfo("Sentinel iniciado correctamente 1.5");

        //auxiliares
        std::thread hilo_json(actualizarJSON, std::ref(config_compartida));

        std::thread hilo_comandos(loopComandos, std::ref(config_compartida));

        //sentienl
        std::thread  hilo_backup(loopBackup, std::ref(config_compartida));

        std::thread hilo_monitor(loopMonitor, std::ref(config_compartida));

        std::thread hilo_organizador(ejecutarOrganizador,std::ref(config_compartida));


        hilo_json.join();
        hilo_comandos.join();
        hilo_backup.join();
        hilo_monitor.join();
        hilo_organizador.join();

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