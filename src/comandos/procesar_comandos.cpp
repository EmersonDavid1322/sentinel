#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <sys/stat.h>
#include <fstream>
#include "errores.h"
#include "logger.h"
#include "sentinel_estado.h"
#include "rutas.h"
#include "comandos_auxiliar.h"
#include "config_compartida.h"
#include "comandos_backup.h"
#include "comandos_monitor.h"
#include "comandos_organizador.h"
#include "comandos_estado.h"

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

//backup
void procesarComandoBackup(std::string& accion, std::string& valor, const ConfigBackup& configBackup){
    if (accion == "activar" || accion == "desactivar"){
        procesarEstado("backup", accion);
    }
    else if (accion == "añadir_carpeta") {
        agregarCarpetaBackup(valor);
    }
    else if (accion == "destino") {
        cambiarDireccion("backup",accion ,valor);
    }
    else if (accion == "ahora") {
        ejecutarBackupComando(configBackup);
    }
    else {
        enviarRespuesta("Accion '" + accion + "' no disponible en el modulo de backup");
    }
}

//monitor
void procesarComandoMonitor(std::string& accion, std::string& valor){
    if (accion == "activar" || accion == "desactivar"){
        procesarEstado("monitor", accion);
    }
    else if (accion == "limite_cpu" || accion == "limite_ram" || accion == "limite_disco" ){
        ejecutarCambioValorMonitor(accion, valor);
    }
    else if (accion == "consumo") {
        ejecutarMonitoreoComando();
    }
    else{
        enviarRespuesta("Accion '" + accion + "' no disponible en el modulo de monitor");
    }
}

//organizador
void procesarComandoOrganizador(std::string& accion, std::string& valor){
    if (accion == "activar" || accion == "desactivar"){
        procesarEstado("organizador", accion);
    }
    else if (accion == "carpeta_vigilar") {
        cambiarDireccion("organizador",accion, valor);
    }
    else if (accion == "agregar_regla") {
        procesarComandoOrganizadorAgregarRegla(valor);
    }
}

//estado
void procesarComandoEstado(std::string& accion, const ConfigSentinel& config) {
    if (accion.empty()) {
        enviarRespuesta(generarDiagnostico(config));
    }
    else if (accion == "backup") {
        enviarRespuesta(estadoBackup(config.backup));
    }
    else if (accion == "monitor") {
        enviarRespuesta(estadoMonitor(config.monitor));
    }
    else if (accion == "organizador") {
        enviarRespuesta(estadoOrganizador(config.organizador));
    }
}

void procesarComando(const std::string& comando, const ConfigSentinel& config) {
    std::istringstream stream(comando);
    std::string modulo, accion, valor;
    stream >> modulo >> accion;
    std::getline(stream, valor);

    if (modulo == "backup") {
        procesarComandoBackup(accion, valor, config.backup);
    } else if (modulo == "monitor") {
        procesarComandoMonitor(accion, valor);
    } else if (modulo == "organizador") {
        procesarComandoOrganizador(accion, valor);
    } else if (modulo == "estado") {
        procesarComandoEstado(accion, config);
    } else {
        enviarRespuesta("Módulo no reconocido: " + modulo);
    }
}

void loopComandos(ConfigCompartida& config_compartida) {
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
        ConfigSentinel config = config_compartida.obtener();
        int resultado = poll(&pfd, 1, 1000);

        if (resultado > 0 && (pfd.revents & POLLIN)) {
            char buffer[256]; 
            int bytes = read(fd, buffer, sizeof(buffer) - 1);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                std::string comando(buffer);

                if (!comando.empty() && comando.back() == '\n'){
                    comando.pop_back();
                }
                procesarComando(comando, config);
            }
        }
    }
    close(fd);
}