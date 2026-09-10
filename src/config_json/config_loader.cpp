#include "config_loader.h"
#include <string>
#include <vector>
#include "json.hpp"
#include "errores.h"
#include "sentinel_config.h"
#include "notificador.h"
using json = nlohmann::json;

ConfigBackup cargarBackup(const json& datos) {

    json plantilla_backup = R"({
        "backup": {
            "carpetas": [],
            "destino": "",
            "ignorar": [],
            "solo_modificados_hoy": false,
            "hora": "00:00",
            "activo": false,
            "forzar_backup": false,
            "crear_carpeta_backup": false,
            "eliminar_ultimo_backup_registrado": false
        }
    })"_json;

    plantilla_backup.merge_patch(datos);

    ConfigBackup backup;
    backup.carpetas             = plantilla_backup["backup"]["carpetas"];
    backup.destino              = plantilla_backup["backup"]["destino"];
    backup.ignorar              = plantilla_backup["backup"]["ignorar"];
    backup.solo_modificados_hoy = plantilla_backup["backup"]["solo_modificados_hoy"];
    backup.hora                 = plantilla_backup["backup"]["hora"];
    backup.activo               = plantilla_backup["backup"]["activo"];
    backup.forzar_backup        = plantilla_backup["backup"]["forzar_backup"];
    backup.crear_carpeta_backup = plantilla_backup["backup"]["crear_carpeta_backup"];
    backup.eliminar_ultimo_backup_registrado = plantilla_backup["backup"]["eliminar_ultimo_backup_registrado"];

    return backup;
}

ConfigBackupNube cargarBackupNube(const json& datos) {

    json plantilla_nube = R"({
        "backup_nube": {
            "carpetas": [],
            "carpeta_remota": "/",
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
            "crear_carpeta_backup_nube": false,
            "eliminar_ultimo_backup_registrado": false
        }
    })"_json;

    plantilla_nube.merge_patch(datos);

    ConfigBackupNube nube;
    nube.carpetas                  = plantilla_nube["backup_nube"]["carpetas"];
    nube.carpeta_remota            = plantilla_nube["backup_nube"]["carpeta_remota"];
    nube.carpeta_destino           = plantilla_nube["backup_nube"]["carpeta_destino"];
    nube.ignorar                   = plantilla_nube["backup_nube"]["ignorar"];
    nube.token                     = plantilla_nube["backup_nube"]["token"];
    nube.clienteID                 = plantilla_nube["backup_nube"]["cliente_id"];
    nube.clienteSecret             = plantilla_nube["backup_nube"]["cliente_secret"];
    nube.refresh_token             = plantilla_nube["backup_nube"]["refresh_token"];
    nube.hora                      = plantilla_nube["backup_nube"]["hora"];
    nube.hora_bajada               = plantilla_nube["backup_nube"]["hora_bajada"];
    nube.activo                    = plantilla_nube["backup_nube"]["activo"];
    nube.activo_bajada             = plantilla_nube["backup_nube"]["activo_bajada"];
    nube.solo_subir_modificados_hoy = plantilla_nube["backup_nube"]["solo_subir_modificados_hoy"];
    nube.crear_carpeta_backup_nube = plantilla_nube["backup_nube"]["crear_carpeta_backup_nube"];

    return nube;
}

ConfigMonitor cargarMonitor(const json& datos){

    json plantilla_monitor = R"({
        "monitor": {
            "intervalo": 15,
            "limite_cpu": 70,
            "limite_ram": 70,
            "limite_disco": 70,
            "activo": false
        }
    })"_json;

    plantilla_monitor.merge_patch(datos);

    ConfigMonitor monitor;
    monitor.intervalo = plantilla_monitor["monitor"]["intervalo"];
    monitor.cpu       = plantilla_monitor["monitor"]["limite_cpu"];
    monitor.ram       = plantilla_monitor["monitor"]["limite_ram"];
    monitor.disco     = plantilla_monitor["monitor"]["limite_disco"];
    monitor.activo    = plantilla_monitor["monitor"]["activo"];

    return monitor;
}

ConfigOrganizador cargarOrganizador(const json& datos){

    json plantilla_organizador = R"({
        "organizador": {
            "carpeta_vigilar": "",
            "activo": false,
            "reglas": {}
        }
    })"_json;

    plantilla_organizador.merge_patch(datos);

    ConfigOrganizador organizador;
    organizador.carpeta_vigilar = plantilla_organizador["organizador"]["carpeta_vigilar"];
    organizador.activo          = plantilla_organizador["organizador"]["activo"];
    organizador.reglas          = plantilla_organizador["organizador"]["reglas"];

    return organizador;
}

ConfigSentinel cargarConfig(const std::filesystem::path& rutaJSON){
    try{
        std::ifstream archivo = comprobar_json(rutaJSON);
        json datos = json::parse(archivo);

        ConfigBackup struct_backup = cargarBackup(datos);
        ConfigBackupNube struc_backup_nube = cargarBackupNube(datos);
        ConfigMonitor struct_monitor = cargarMonitor(datos);
        ConfigOrganizador struct_organizador = cargarOrganizador(datos);

        ConfigSentinel config;
        config.backup = struct_backup;
        config.backup_nube = struc_backup_nube;
        config.monitor = struct_monitor;
        config.organizador = struct_organizador;

        return config;
    }
    catch (const nlohmann::json::exception& e) {
        throw ErrorConfig("El archivo de configuración tiene un error: " + std::string(e.what()));
        enviarNotificación("Error config", "Archivo configuraciones corrupto" + std::string(e.what()), "ERROR");
    }
}