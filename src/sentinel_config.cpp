#include <string>
#include <filesystem>
#include <sys/inotify.h>
#include <iostream>
#include <unistd.h>
#include "sentinel_config.h"
#include "config_loader.h"
#include "errores.h"
#include "logger.h"
#include "rutas.h"
namespace fs = std::filesystem;
fs::path ruta = (obtenerRutaBase() / "config" / "sentinel.json");

void actualizarJSON(){
    while (true){
        int fd = inotify_init();
        int wd = inotify_add_watch(fd, ruta.c_str(), IN_MODIFY);

        char buffer[4096];
        int bytes = read(fd, buffer, sizeof(buffer));
        if (bytes < 0) break;

        struct inotify_event* evento = (struct inotify_event*) buffer;

        if (evento->mask & IN_MODIFY){
            logInfo("JSON modificado, reiniciando Sentinel...");
            int resultado = system("systemctl --user restart sentinel.service");
            if (resultado != 0) {
                logError("Error al reiniciar Sentinel: código " + std::to_string(resultado));
            }
        }

        inotify_rm_watch(fd, wd);
        close(fd);
    }
}