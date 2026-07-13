#pragma once
#include <sys/inotify.h>
#include <unistd.h>
#include "errores.h"

class VigilanteInotify {
public:
    int fd;
    int wd;

    VigilanteInotify(const std::string& ruta, uint32_t eventos) {
        fd = inotify_init();
        if (fd == -1) {
            throw ErrorBackup("No se pudo inicializar inotify");
        }

        wd = inotify_add_watch(fd, ruta.c_str(), eventos);
        if (wd == -1) {
            close(fd);
            throw ErrorBackup("No se pudo vigilar la ruta: " + ruta);
        }
    }

    ~VigilanteInotify() {
        inotify_rm_watch(fd, wd);
        close(fd);
    }
};