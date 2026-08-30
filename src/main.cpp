#include <filesystem>
#include <thread>
#include <curl/curl.h>
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
#include "loop_backup_nube.h"
#include "notificador.h"
namespace fs = std::filesystem;

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    verficarEntornoGrafico();

    if (hayEntornoGrafico) {
        notify_init("Sentinel");
        logInfo("Se a detectado entorno grafico. Notificaciones activadas", "sentinel.log");
    }else {
        logInfo("No se detectó entorno gráfico. Las notificaciones quedan desactivadas (modo servidor)", "sentinel.log");
    }
    capturarSenal();
    
    std::filesystem::path rutaConfig = obtenerRutaConfig();

    try {
        asegurarConfigExiste(rutaConfig);
        ConfigCompartida config_compartida;
        config_compartida.actualizar(cargarConfig(rutaConfig));
        logInfo("Sentinel iniciado correctamente 1.86", "sentinel.log");
        enviarNotificación("Inicio", "Sentinel iniciado correctamente 1.86", "INFO");

        //auxiliares
        std::thread hilo_json(actualizarJSON, std::ref(config_compartida));

        std::thread hilo_comandos(loopComandos, std::ref(config_compartida));

        //sentienl
        std::thread  hilo_backup(loopBackup, std::ref(config_compartida));

        std::thread hilo_backupNube(loopBackupNube ,std::ref(config_compartida));

        std::thread hilo_monitor(loopMonitor, std::ref(config_compartida));

        std::thread hilo_organizador(ejecutarOrganizador,std::ref(config_compartida));


        hilo_json.join();
        hilo_comandos.join();
        hilo_backup.join();
        hilo_backupNube.join();
        hilo_monitor.join();
        hilo_organizador.join();

    } catch (const DaemonError& e) {
        curl_global_cleanup();
        logError("Error critico al iniciar: " + std::string(e.what()), "sentinel.log");
        if (hayEntornoGrafico) {
            notify_uninit();
        }
        logInfo("Sentinel detenido correctamente", "sentinel.log");
        return 1;
    }

    if (hayEntornoGrafico) {
        notify_uninit();
    }
    curl_global_cleanup();
    logInfo("Sentinel detenido correctamente", "sentinel.log");
    return 0;
}