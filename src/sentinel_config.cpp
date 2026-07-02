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
namespace fs = std::filesystem;
fs::path ruta = (obtenerRutaBase() / "config" / "sentinel.json");

void actualizarJSON(){
    int fd = inotify_init();
    int wd = inotify_add_watch(fd, ruta.c_str(), IN_MODIFY);

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    while (corriendo){
        int resultado = poll(&pfd, 1, 1000);

        if (resultado < 0) break;

        if (pfd.revents & POLLIN) {
            char buffer[4096];
            int bytes = read(fd, buffer, sizeof(buffer));
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
    inotify_rm_watch(fd, wd);
    close(fd);
}
