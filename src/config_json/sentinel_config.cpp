#include <string>
#include <filesystem>
#include <sys/inotify.h>
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
#include "json.hpp"
namespace fs = std::filesystem;
using json = nlohmann::json;

std::ifstream comprobar_json(const std::filesystem::path& ruta){
    std::ifstream archivo(ruta);
    if (!archivo.is_open()){
        throw ErrorConfig("El archivo json no se a podido encontrar: " + ruta.string());
    }
    return archivo;
}

void actualizarJSON(ConfigCompartida& configCompartida){
    try{
        fs::path ruta = obtenerRutaConfig();
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
                if (bytes < 0) break;

                for (int i = 0; i < bytes; ) {
                    struct inotify_event* evento = (struct inotify_event*)&buffer[i];
                    if (evento->mask & IN_MODIFY){
                        configCompartida.actualizar(cargarConfig(ruta));
                        logInfo("Sentinel actualizado correctamente", "sentinel.log");
                    }
                    i += sizeof(struct inotify_event) + evento->len;
                }
            }
        }
    }
    catch(const ErrorInotify& e){
        logError("Error en Deamon - " + std::string(e.what()), "sentinel.log");
        enviarNotificación("Error Error intify-JSON", "Ocurrio un error en el vigilante del JSON: " + std::string(e.what()), "ERROR");
    }
    catch(const DaemonError& e){
        logError("Error en Deamon - " + std::string(e.what()), "sentinel.log");
        enviarNotificación("Error Deamon-JSON", "Ocurrio un error en el vigilante del JSON: " + std::string(e.what()), "ERROR");
    }
}

json obtenerConfiguracionPorDefectoComplete() {
    return R"({
        "backup": {
            "carpetas": [],
            "destino": "",
            "ignorar": [],
            "solo_modificados_hoy": false,
            "hora": "00:00",
            "activo": false,
            "forzar_backup": false,
            "crear_carpeta_backup": false
        },
        "backup_nube": {
            "carpetas": [],
            "carpeta_remota": "",
            "ignorar": [],
            "token": "",
            "cliente_id": "",
            "cliente_secret": "",
            "refresh_token": "",
            "hora": "00:00",
            "hora_bajada": "00:00",
            "carpeta_destino": "",
            "activo": false,
            "activo_bajada": false,
            "solo_subir_modificados_hoy": false,
            "crear_carpeta_backup_nube": false
        },
        "monitor": {
            "intervalo": 0,
            "limite_cpu": 0,
            "limite_ram": 0,
            "limite_disco": 0,
            "activo": false
        },
        "organizador": {
            "carpeta_vigilar": "",
            "activo": false,
            "reglas": {}
        }
    })"_json;
}

void inicializarConfiguraciones() {
    json config_base = obtenerConfiguracionPorDefectoComplete();
    fs::path rutaConfig = obtenerRutaConfig();
    json config_usuario;

    std::ifstream archivoConfig(rutaConfig);

    if (!archivoConfig.is_open()) {
        logError("No se puedo abrir el archivo de configuraciones en: " + rutaConfig.string(), "sentinel.log");
    }

    try {
        archivoConfig >> config_usuario;
        config_base.merge_patch(config_usuario);
    } catch (const json::parse_error& e) {
        logError("Error al leer el JSON (archivo corrupto). Se usará el defecto. " + std::string(e.what()), "sentinel.log");
    }
    archivoConfig.close();

    std::ofstream archivo_escritura(rutaConfig);
    if (archivo_escritura.is_open()) {
        archivo_escritura << config_base.dump(4);
        archivo_escritura.close();
    }
}

void crearConfigPorDefecto(const std::filesystem::path& rutaJSON){
    using json = nlohmann::json;

    json config;
    config["backup"] = {};
    config["backup"]["carpetas"] = std::vector<std::string>{};
    config["backup"]["destino"] = "";
    config["backup"]["ignorar"] = std::vector<std::string>{};
    config["backup"]["solo_modificados_hoy"] = false;
    config["backup"]["hora"] = "00:00";
    config["backup"]["activo"] = false;
    config["backup"]["forzar_backup"] = false;
    config["backup"]["crear_carpeta_backup"] = false;

    config["backup_nube"] = {};
    config["backup_nube"]["carpetas"] = std::vector<std::string>{};
    config["backup_nube"]["carpeta_remota"] = "/";
    config["backup_nube"]["ignorar"] = std::vector<std::string>{};
    config["backup_nube"]["token"] = "";
    config["backup_nube"]["cliente_id"] = "";
    config["backup_nube"]["cliente_secret"] = "";
    config["backup_nube"]["refresh_token"] = "";
    config["backup_nube"]["hora"] = "00:00";
    config["backup_nube"]["hora_bajada"] = "00:00";
    config["backup_nube"]["carpeta_destino"] = "";
    config["backup_nube"]["activo"] = false;
    config["backup_nube"]["activo_bajada"] = false;
    config["backup_nube"]["solo_subir_modificados_hoy"] = false;
    config["backup_nube"]["crear_carpeta_backup_nube"] = false;

    config["monitor"] = {};
    config["monitor"]["intervalo"] = 60;
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
        logInfo("No se encontro la carpeta 'config' se creo una nueva: " + carpeta_padre.string(), "sentinel.log");
    }

    if (!fs::exists(rutaJSON)){
        crearConfigPorDefecto(rutaJSON);
        logInfo("No se encontro el archivo 'sentinel.json' se creo uno nuevo: " + rutaJSON.string() +
            " se recomienda proporcinarle y verificar los permisos correctos", "sentinel.log");
    }
}
