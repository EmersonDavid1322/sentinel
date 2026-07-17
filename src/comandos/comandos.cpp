#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <sys/stat.h>
#include "comandos.h"
#include <fstream>
#include "errores.h"
#include "logger.h"
#include "sentinel_estado.h"
#include "rutas.h"
#include "comandos_deamon.h"
namespace fs = std::filesystem;

void enviarRespuesta(const std::string& mensaje) {
    std::filesystem::path ruta_estado = obtenerRutaBase() / "sentinel_estado.txt";
    std::ofstream salida(ruta_estado);
    if (salida.is_open()) {
        salida << mensaje << std::endl;
    }
}

void procesarComando(const std::string& comando) {
    std::istringstream stream(comando);
    std::string accion, seccion;
    stream >> accion >> seccion;
    
    if (accion == "activar" || accion == "desactivar") {
        if (seccion != "backup" && seccion != "monitor" && seccion != "organizador") {
            enviarRespuesta("La sección '" + seccion + "' no existe. Usa: backup, monitor u organizador.");
            return;
        }
    }

    if (accion == "estado") {
        enviarRespuesta("Sentinel ejecutandose correctamente");
    }
    else if (accion == "activar"){
        cambiarEstadoSeccion(seccion, true);
        enviarRespuesta("Seccion: " + seccion + " activada");
    }
    else if (accion == "desactivar"){
        cambiarEstadoSeccion(seccion, false);
        enviarRespuesta("Seccion: " + seccion + " desactivada");
    }
    else {
        enviarRespuesta("Comando no reconociodo: " + comando);
    }
}

void loopComandos() {
    std::string ruta_fifo = (obtenerRutaBase() / "sentinel.fifo").string();

    mkfifo(ruta_fifo.c_str(), 0666);

    int fd = open(ruta_fifo.c_str(), O_RDWR);
    if (fd == -1) {
        throw ErrorConfig("No se pudo abrir el FIFO de comandos en: " + ruta_fifo);
    }

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    while (corriendo) {
        int resultado = poll(&pfd, 1, 1000);

        if (resultado > 0 && (pfd.revents & POLLIN)) {
            char buffer[256]; 
            int bytes = read(fd, buffer, sizeof(buffer) - 1);
            std::cout << "poll detecto actividad, bytes leidos: " << bytes << std::endl;
            if (bytes > 0) {
                buffer[bytes] = '\0';
                std::string comando(buffer);

                if (!comando.empty() && comando.back() == '\n'){
                    comando.pop_back();
                }
                procesarComando(comando);
            }
        }
    }
    close(fd);
}