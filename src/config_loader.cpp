#include <iostream>
#include <string>
#include <vector>
#include "config_loader.h"
#include "json.hpp"
#include "errores.h"
using json = nlohmann::json;

std::ifstream comprobar_json(const std::filesystem::path& ruta){
    std::ifstream archivo(ruta);
    if (!archivo.is_open()){
        throw DaemonError("El archivo json no se a podido encontrar: " + ruta.string());
    }
    return archivo;
}

ConfigBackup cargarBackup(const json& datos){

    std::vector<std::string> carpetas = datos["backup"]["carpetas"];
    std::string destino = datos["backup"]["destino"];
    std::string hora = datos["backup"]["hora"];
    bool activo = datos["backup"]["activo"];

    ConfigBackup backup;
    backup.carpetas = carpetas;
    backup.destino = destino;
    backup.hora = hora;
    backup.activo = activo;

    return backup;
}

ConfigMonitor cargarMonitor(const json& datos){

    int cpu = datos["monitor"]["limite_cpu"];
    int ram = datos["monitor"]["limite_ram"];
    int disco = datos["monitor"]["limite_disco"];
    bool activo = datos["monitor"]["activo"];

    ConfigMonitor monitor;
    monitor.cpu = cpu;
    monitor.ram = ram;
    monitor.disco = disco;
    monitor.activo = activo;

    return monitor;
}

ConfigOrganizador cargarOrganizador(const json& datos){

    std::string carpeta_vigilar = datos["organizador"]["carpeta_vigilar"];
    bool activado = datos["organizador"]["activo"];
    std::map<std::string, std::string> reglas = datos["organizador"]["reglas"];

    ConfigOrganizador organizador;
    organizador.carpeta_vigilar = carpeta_vigilar;
    organizador.activo = activado;
    organizador.reglas = reglas;

    return organizador;
}

ConfigSentinel cargarConfig(const std::filesystem::path& rutaJSON){
    try{
        std::ifstream archivo = comprobar_json(rutaJSON);
        json datos = json::parse(archivo);

        ConfigBackup struct_backup = cargarBackup(datos);
        ConfigMonitor struct_monitor = cargarMonitor(datos);
        ConfigOrganizador struct_organizador = cargarOrganizador(datos);

        ConfigSentinel config;
        config.backup = struct_backup;
        config.monitor = struct_monitor;
        config.organizador = struct_organizador;

        return config;
    }
    catch (const nlohmann::json::exception& e) {
        throw ErrorConfig("El archivo de configuración tiene un error: " + std::string(e.what()));
    }
}