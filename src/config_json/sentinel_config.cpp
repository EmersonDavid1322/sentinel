#include <string>
#include <filesystem>
#include <sys/inotify.h>
#include <sys/wait.h>
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

std::ifstream comprobar_json(const std::filesystem::path& ruta){
    std::ifstream archivo(ruta);
    if (!archivo.is_open()){
        throw ErrorConfig("El archivo json no se a podido encontrar: " + ruta.string());
    }
    return archivo;
}

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
                if (WIFEXITED(resultado)){
                    int codigo_salida = WEXITSTATUS(resultado);
                    if (codigo_salida == 0){
                        logInfo("Señal de reinicio recibida correctamente");
                    } else{
                        logError("Error al reinicar el Sentinel: código " + std::to_string(codigo_salida));
                    }
                } else if (WIFSIGNALED(resultado)){
                    int señal = WTERMSIG(resultado);
                    logError("El comando de reinicio fue terminado por la señal: " + std::to_string(señal));
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

void crearConfigPorDefecto(const std::filesystem::path& rutaJSON){
    using json = nlohmann::json;

    json config;
    config["backup"] = {};
    config["backup"]["_notas_carpetas"] = "Sin / al final para copiar la carpeta completa, Con / para copiar el interior de una carpeta";
    config["backup"]["carpetas"] = std::vector<std::string>{};
    config["backup"]["destino"] = "";
    config["backup"]["hora"] = "00:00";
    config["backup"]["activo"] = false;

    config["monitor"] = {};
    config["monitor"]["limite_cpu"] = 70;
    config["monitor"]["limite_ram"] = 70;
    config["monitor"]["limite_disco"] = 70;
    config["monitor"]["activo"] = false;

    config["organizador"] = {};
    config["organizador"]["carpeta_vigilar"] = "";
    config["organizador"]["activo"] = false;
    config["organizador"]["reglas"] = json::object();

    std::ofstream archivo(rutaJSON);
    if (!archivo.is_open()){
        throw ErrorConfig("No se pudo crear el archivo de configuración en: " + rutaJSON.string());
    }
    archivo << config.dump(4);
}

void asegurarConfigExiste(const std::filesystem::path& rutaJSON){
    std::filesystem::path carpeta_padre = rutaJSON.parent_path();

    if (!fs::exists(carpeta_padre)){
        fs::create_directories(carpeta_padre);
        logInfo("No se encontro la carpeta 'config' se creo una nueva: " + carpeta_padre.string());
    }

    if (!fs::exists(rutaJSON)){
        crearConfigPorDefecto(rutaJSON);
        logInfo("No se encontro el archivo 'sentinel.json' se creo uno nuevo: " + rutaJSON.string());
    }
}