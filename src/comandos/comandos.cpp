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
    std::filesystem::path ruta_estado = obtenerRutaBase() / "config" / "sentinel_estado.txt";
    std::ofstream salida(ruta_estado);
    if (salida.is_open()) {
        salida << mensaje << std::endl;
    }else{
        logError("No se pudo abrir el archivo sentinel_estado.txt en: " + ruta_estado.string());
    }
}

void procesarEstado(std::string modulo, std::string& accion){
    if (accion == "activar") {
        cambiarEstadoSeccion(modulo, true);
        }
    else if (accion == "desactivar"){
        cambiarEstadoSeccion(modulo, false);
    }
    enviarRespuesta("Se lanzo el comando: "+ accion + " al modulo: " + modulo);
}

void procesarComandoBackup(std::string& accion, std::string& valor){
    if (accion == "activar" || accion == "desactivar"){
        procesarEstado("backup", accion);
    }
}

void procesarComandoMonitor(std::string& accion, std::string& valor){
    if (accion == "activar" || accion == "desactivar"){
        procesarEstado("monitor", accion);
    }
    else if (accion == "limite_cpu" || accion == "limite_ram" || accion == "limite_disco" ){
        try{
            double valor_double = std::stod(valor);
            if (valor_double < 0 || valor_double > 100){
                enviarRespuesta("El límite debe estar entre 0 y 100.");
                return;
            }
            cambiarValorMonitor(accion, valor_double);
            enviarRespuesta("Se cambio el valor del parametro " + accion + ":" + valor);
        }catch (const std::invalid_argument&){
            enviarRespuesta("El valor '" + valor + "' no es un numero valido");
        }catch (const std::out_of_range&){
            enviarRespuesta("El valor '" + valor + "' es demasiado grande.");
        }
    }
    else{
        enviarRespuesta("Accion '" + accion + "' no disponible en el modulo de monitor");
    }
}

void procesarComandoOrganizador(std::string& accion, std::string& valor){
    if (accion == "activar" || accion == "desactivar"){
        procesarEstado("organizador", accion);
    }
}

void procesarComando(const std::string& comando) {
    std::istringstream stream(comando);
    std::string modulo, accion, valor;
    stream >> modulo >> accion;
    std::getline(stream, valor);

    if (modulo == "monitor") {
        procesarComandoMonitor(accion, valor);
    } else if (modulo == "backup") {
        procesarComandoBackup(accion, valor);
    } else if (modulo == "organizador") {
        procesarComandoOrganizador(accion, valor);
    } else if (modulo == "estado") {
        enviarRespuesta("Sentinel ejecutandose correctamente");
    } else {
        enviarRespuesta("Módulo no reconocido: " + modulo);
    }
}

void loopComandos() {
    std::string ruta_fifo = (obtenerRutaBase() / "config" / "sentinel.fifo").string();
    std::string ruta_estado = obtenerRutaBase() / "config" / "sentinel_estado.txt";

    mkfifo(ruta_fifo.c_str(), 0666);
    std::ofstream salida(ruta_estado);

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