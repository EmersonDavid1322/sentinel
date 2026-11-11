#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <sys/stat.h>
#include <fstream>
#include <sys/socket.h>
#include <poll.h>
#include <sys/un.h>
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
#include "comandos_backup_nube.h"
#include "backup.h"
#include "backup_nube_subida.h"

std::string procesarEstado(const std::string& modulo, const std::string& accion){
    try {
        if (accion == "activar") {
            cambiarEstadoSeccion(modulo, true);
        }
        else if (accion == "desactivar"){
            cambiarEstadoSeccion(modulo, false);
        }
        return "Se cambio el estado del modulo "+ modulo + " a: " + accion;
    }
   catch (const ErrorConfig& e) {
       logError("Ocurrio un error al intentar cambiar de estado el modulo: " + modulo + " Error: " + std::string(e.what()), "sentinel.log");
        return "Ocurrio un error al intentar cambiar de estado el modulo: " + modulo + " Error: " + std::string(e.what());
   }
}

//backup_local
std::string procesarComandoBackup(const std::string& accion, const std::string& valor, const ConfigBackup& configBackup, const ConfigMonitor& configMonitor){
    if (accion == "activar" || accion == "desactivar"){
        return procesarEstado("backup", accion);
    }
    else if (accion == "añadir_carpeta") {
        return agregarCarpetaBackup(valor);
    }
    else if (accion == "destino") {
        return cambiarDireccion("backup",accion ,valor);
    }
    else if (accion == "ahora") {
        logInfo("Se inicio el backup local por comando ", "sentinel.log");
        ResultadoHacerBackup respuesta = hacerBackup(configBackup, configMonitor);
        return mensajeResultadoBackup(respuesta);
    }
    else if (accion == "modo") {
        return cambiarForzarBackup(valor);
    }
    else if (accion == "ignorar") {
        return aniadirIgnorar("backup", valor);
    }
    else {
       return "Accion '" + accion + "' no disponible en el modulo de backup local";
    }
}

//backup_nube
std::string procesarComandoBackupNube(const std::string& accion, const std::string& valor, const ConfigBackupNube& config) {
    if (accion == "activar" || accion == "desactivar") {
        return procesarEstado("backup_nube", accion);
    }
    else if (accion == "lista_nube") {
        return mostrarListadoComando(config);
    }
    else if (accion == "ignorar") {
        return aniadirIgnorar("backup_nube", valor);
    }
    else if (accion == "ahora") {
        if (corriendo_backup_nube) {
            return "Actualmente hay un backup a nube ejecutandose mediante la configuraciones del json";
        }
        logInfo("Se inicio el backup a dropbox por comando ", "sentinel.log");
        ejecutarBackupNube(config);
        return "Backup nube completo";
    }
    else{
        return "Accion '" + accion + "' no disponible en el modulo de backup nube";
    }
}

//monitor
std::string procesarComandoMonitor(std::string& accion, std::string& valor){
    if (accion == "activar" || accion == "desactivar"){
        return procesarEstado("monitor", accion);
    }
    else if (accion == "limite_cpu" || accion == "limite_ram" || accion == "limite_disco" ){
        return ejecutarCambioValorMonitor(accion, valor);
    }
    else if (accion == "consumo") {
        return ejecutarMonitoreoComando();
    }
    else{
        return "Accion '" + accion + "' no disponible en el modulo de monitor";
    }
}

//organizador
std::string procesarComandoOrganizador(const std::string& accion, const std::string& valor){
    if (accion == "activar" || accion == "desactivar"){
        return procesarEstado("organizador", accion);
    }
    else if (accion == "carpeta_vigilar") {
        return cambiarDireccion("organizador",accion, valor);
    }
    else if (accion == "agregar_regla") {
        return  procesarComandoOrganizadorAgregarRegla(valor);
    }
    else {
        return "Accion '" + accion + "' no disponible en el modulo de organizador";
    }
}

//estado
std::string procesarComandoEstado(const std::string& accion, const ConfigSentinel& config) {
    if (accion.empty()) {
        return generarDiagnostico(config);
    }
    else if (accion == "backup") {
        return estadoBackup(config.backup);
    }
    else if (accion == "backup_nube") {
        return estadoBackupNube(config.backup_nube);
    }
    else if (accion == "monitor") {
        return estadoMonitor(config.monitor);
    }
    else if (accion == "organizador") {
        return estadoOrganizador(config.organizador);
    }
    else {
        return "No se a encontrado: " + accion;
    }
}

std::string procesarComando(const std::string& comando, const ConfigSentinel& config) {
    std::istringstream stream(comando);
    std::string modulo, accion, valor;
    stream >> modulo >> accion;
    std::getline(stream, valor);

    if (modulo == "backup") {
        return procesarComandoBackup(accion, valor, config.backup, config.monitor);
    }else if (modulo == "backup_nube") {
        return procesarComandoBackupNube(accion, valor, config.backup_nube);
    } else if (modulo == "monitor") {
        return procesarComandoMonitor(accion, valor);
    } else if (modulo == "organizador") {
        return procesarComandoOrganizador(accion, valor);
    } else if (modulo == "estado") {
        return procesarComandoEstado(accion, config);
    } else {
        return "Módulo no reconocido: " + modulo;
    }
}

void loopComandos(ConfigCompartida& config_compartida) {
    std::string ruta_socket = (obtenerRutaEstado() / "sentinel.sock").string();

    //Crear el socket Unix
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        throw ErrorConfig("No se pudo crear el socket en: " + ruta_socket);
    }

    unlink(ruta_socket.c_str());

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, ruta_socket.c_str(), sizeof(addr.sun_path) - 1);

    // Vincular y activar
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(server_fd);
        throw ErrorConfig("No se pudo hacer bind en: " + ruta_socket);
    }
    listen(server_fd, 5);

    // Poll
    struct pollfd pfd;
    pfd.fd = server_fd;
    pfd.events = POLLIN;

    while (corriendo) {
        ConfigSentinel config = config_compartida.obtener();
        int resultado = poll(&pfd, 1, 1000);

        if (resultado > 0 && (pfd.revents & POLLIN)) {
            //Aceptar la conexión entrante del cliente
            int client_fd = accept(server_fd, nullptr, nullptr);
            if (client_fd != -1) {
                char buffer[256];
                int bytes = read(client_fd, buffer, sizeof(buffer) - 1);

                if (bytes > 0) {
                    buffer[bytes] = '\0';
                    std::string comando(buffer);

                    if (!comando.empty() && comando.back() == '\n') {
                        comando.pop_back();
                    }
                    std::string respuesta_comando = procesarComando(comando, config);
                    write(client_fd, respuesta_comando.c_str(), respuesta_comando.size());
                }
                // Cerrar el socket
                close(client_fd);
            }
        }
    }
    close(server_fd);
    unlink(ruta_socket.c_str());
}
