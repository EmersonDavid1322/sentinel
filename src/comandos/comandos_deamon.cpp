#include <fstream>
#include <string>
#include <filesystem>
#include "json.hpp"
#include "comandos_deamon.h"
#include "errores.h"
#include "logger.h"
#include "rutas.h"
#include "comandos.h"
#include "monitor.h"
#include "backup.h"
using json = nlohmann::json;

//json carga y guarda
json leerJSONActual(const std::filesystem::path& ruta){
    std::ifstream archivo(ruta);
    if (!archivo.is_open()){
        enviarRespuesta("No se pudo abir el archivo configuraciones en la ruta: " + ruta.string());
        throw ErrorConfig("No se pudo abir el archivo configuraciones en la ruta: " + ruta.string());
    }
    return json::parse(archivo);
}

void guardarJSON(const json& datos, const std::filesystem::path& ruta){
    std::ofstream archivo(ruta);
    if (!archivo.is_open()){
        enviarRespuesta("No se pudo abir el archivo configuraciones en la ruta: " + ruta.string());
        throw ErrorConfig("No se pudo abir el archivo configuraciones en la ruta: " + ruta.string());
    }
    archivo << datos.dump(4);
}

//auxiliar
std::string limpiarEspacios(const std::string& texto) {
    size_t inicio = texto.find_first_not_of(" \t\r\n");
    if (inicio == std::string::npos) {
        return "";
    }
    size_t final = texto.find_last_not_of(" \t\r\n");
    return texto.substr(inicio, final - inicio + 1);
}

//cambiar estado acitvo modulo
void cambiarEstadoSeccion(const std::string& seccion, bool activo){
    std::filesystem::path ruta = obtenerRutaBase() / "config" / "sentinel.json";

    json datos = leerJSONActual(ruta);
    datos [seccion]["activo"] = activo;
    guardarJSON(datos, ruta);
}

//comandos backup
void agregarCarpetaBackup(const std::string& carpeta) {

    std::string carpeta_limpia = limpiarEspacios(carpeta);
    if (carpeta.empty()) {
        enviarRespuesta("No se permiten valores vacios en agregar carpeta");
        return;
    }

    std::filesystem::path ruta = obtenerRutaBase() / "config" / "sentinel.json";
    json datos = leerJSONActual(ruta);

    std::vector<std::string> carpetas = datos["backup"]["carpetas"];
    carpetas.push_back(carpeta_limpia);

    datos["backup"]["carpetas"] = carpetas;

    guardarJSON(datos, ruta);
    enviarRespuesta("Se a añadido la carpeta: " + carpeta_limpia);
}

void cambiarDestinoBackup(const std::string& destino) {
    std::string destino_limpio = limpiarEspacios(destino);
    if (destino_limpio.empty()) {
        enviarRespuesta("No se permiten valores vacios en destino");
        return;
    }

    std::filesystem::path ruta = obtenerRutaBase() / "config" / "sentinel.json";

    json datos = leerJSONActual(ruta);
    datos ["backup"]["destino"] = destino_limpio;
    guardarJSON(datos, ruta);
    enviarRespuesta("Se a cambiado el destino del backup a: " + destino_limpio);
}

void ejecutarBackupComando(const ConfigBackup& configBackup) {
    try {
        std::filesystem::path ruta = obtenerRutaBase() / "config" / "sentinel.json";
        json datos = leerJSONActual(ruta);

        std::string carpetas_msg = verificarCarpetas(configBackup.carpetas, configBackup.destino);
        ejecutarBackup(configBackup.carpetas, configBackup.destino);

        logInfo("Se realizo un backup manual de las carpetas: " + carpetas_msg + " Destino: " + configBackup.destino);
        enviarRespuesta("Backup ejecutado correctamente. Carpetas: " + carpetas_msg + " Destino: " + configBackup.destino);
    }
    catch (const ErrorBackup& e) {
        enviarRespuesta("Error al ejecutar backup: " + std::string(e.what()));
    }
    catch (const DaemonError& e) {
        enviarRespuesta("Error Deamon al ejecutar backup: " + std::string(e.what()));
    }
}

//comandos monitor
void cambiarValorMonitor(const std::string& parametro, double valor){
    std::filesystem::path ruta = obtenerRutaBase() / "config" / "sentinel.json";

    json datos = leerJSONActual(ruta);
    datos ["monitor"][parametro] = valor;
    guardarJSON(datos, ruta);
}

void ejecutarCambioValorMonitor(const std::string& parametro, std::string& valor) {
    try{
        double valor_double = std::stod(valor);
        if (valor_double < 0 || valor_double > 100){
            enviarRespuesta("El límite debe estar entre 0 y 100.");
            return;
        }
        cambiarValorMonitor(parametro, valor_double);
        enviarRespuesta("Se cambio el valor del parametro " + parametro + ":" + valor);
    }catch (const std::invalid_argument&){
        enviarRespuesta("El valor '" + valor + "' no es un numero valido");
    }catch (const std::out_of_range&){
        enviarRespuesta("El valor '" + valor + "' es demasiado grande.");
    }
}

void ejecutarMonitoreoComando() {
    try{
        enviarRespuesta("Resultado del consumo actual\nCPU: " + std::to_string(uso_cpu()) + "%"
                        + "\nRAM: " + std::to_string(uso_ram())+ "%" + "\nDisco: " + std::to_string(uso_disco()) + "%");
    }
    catch(const ErrorMonitor& e){
        enviarRespuesta("Error Monitor Ocurrio un error en el intento de telemetrica: " + std::string(e.what()));
    }
    catch(const DaemonError& e){
        enviarRespuesta("Error Deamon-Monitor Ocurrio un error en el intento de telemetrica: " + std::string(e.what()));
    }
}