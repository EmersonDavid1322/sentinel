#include <string>
#include <filesystem>
#include <sys/inotify.h>
#include <iostream>
#include <unistd.h>
#include <poll.h>
#include "sentinel_config.h"
#include "config_loader.h"
#include "errores.h"
#include "logger.h"
#include "rutas.h"
#include "sentinel_estado.h"
#include "descriptor_inotify.h"
#include "notificador.h"
namespace fs = std::filesystem;
fs::path ruta = (obtenerRutaBase() / "config" / "sentinel.json");

void actualizarJSON(){
    try{
    VigilanteInotify vigilante(ruta.c_str(), IN_MODIFY);

    struct pollfd pfd;
    pfd.fd = vigilante.fd;
    pfd.events = POLLIN;

    while (corriendo){
        int resultado = poll(&pfd, 1, 1000);

        if (resultado < 0) break;

        if (pfd.revents & POLLIN) {
            char buffer[4096];
            int bytes = read(vigilante.fd, buffer, sizeof(buffer));
            struct inotify_event* evento = (struct inotify_event*) buffer;
            if (bytes < 0) break;

            if (evento->mask & IN_MODIFY){
                logInfo("JSON modificado, reiniciando Sentinel...");
                int resultado = system("systemctl --user restart sentinel.service");
                if (resultado == 15 || resultado == 0){
                    logInfo("Señal de reinicio recibida correctamente.");
                }
                else if (resultado != 0) {
                    logError("Error al reiniciar Sentinel: código " + std::to_string(resultado));
                }
            }
        }
    }
    }
    catch(const ErrorInotify& e){
        std::cout << "DaemonError: " << e.what() << std::endl;
        logError("Error en Deamon - " + std::string(e.what()));
        enviarNotificación("Error Error intify-JSON", "Ocurrio un error en el vigilante del JSON: " + std::string(e.what()), "ERROR");
    }
    catch(const DaemonError& e){
        std::cout << "DaemonError: " << e.what() << std::endl;
        logError("Error en Deamon - " + std::string(e.what()));
        enviarNotificación("Error Deamon-JSON", "Ocurrio un error en el vigilante del JSON: " + std::string(e.what()), "ERROR");
    }
}
