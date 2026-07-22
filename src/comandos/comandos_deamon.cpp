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
namespace fs = std::filesystem;

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

void cambiarDireccion(const std::string& parametro,const std::string& llave ,const std::string& dirrecion) {
    std::string destino_limpio = limpiarEspacios(dirrecion);
    if (destino_limpio.empty()) {
        enviarRespuesta("No se permiten valores vacios en destino");
        return;
    }


    std::filesystem::path direccion_path(destino_limpio);
    if (!fs::exists(direccion_path)) {
        enviarRespuesta("La dirrección " + destino_limpio + " no a sido encontrada o no existe");
        return;
    }



    std::filesystem::path ruta = obtenerRutaBase() / "config" / "sentinel.json";

    json datos = leerJSONActual(ruta);

    if (datos[parametro][llave] == destino_limpio) {
        enviarRespuesta("Esta dirección " + destino_limpio + " ya esta asignada a " + parametro + "-" + llave);
        return;
    }

    datos [parametro][llave] = destino_limpio;
    guardarJSON(datos, ruta);
    enviarRespuesta("Se a cambiado el destino del backup a: " + destino_limpio);
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

//organizador
void agregarReglaOrganizador(const std::string& extension, const std::string& carpetaDestino) {

    std::filesystem::path ruta_destino(carpetaDestino);
    if (!fs::exists(ruta_destino)) {
        enviarRespuesta("La carpeta destino " + carpetaDestino + " no existe");
        return;
    }

    std::filesystem::path ruta = obtenerRutaBase() / "config" / "sentinel.json";
    json datos = leerJSONActual(ruta);

    std::map<std::string, std::string> reglas = datos["organizador"]["reglas"];
    reglas[extension] = carpetaDestino;
    datos["organizador"]["reglas"] = reglas;

    guardarJSON(datos, ruta);
    enviarRespuesta("Regla agregada: " + extension + " -> " + carpetaDestino);
}

void procesarComandoOrganizadorAgregarRegla(const std::string& valor) {
    std::istringstream stream(valor);
    std::string extension, direccion;

    std::getline(stream, extension, '|');
    std::getline(stream, direccion, '|');

    extension = limpiarEspacios(extension);
    direccion = limpiarEspacios(direccion);

    if (extension.empty() || direccion.empty()) {
        enviarRespuesta("Formato incorrecto. Usa: extension|direccion (ej: .pdf|/home/usuario/PDFs)");
        return;
    }

    if (extension.front() != '.') {
        enviarRespuesta("La extension debe comenzar con un punto, ej: .pdf");
        return;
    }

    agregarReglaOrganizador(extension, direccion);
}

//estado
std::string  estadoBackup(const ConfigBackup& config) {
    std::string carpetas_str;
    for (const auto& carpeta : config.carpetas) {
        carpetas_str += carpeta + "\n";
    }

    std::string mensaje = "=== Estado Backup ===\n";
    mensaje += "Activo: " + std::string(config.activo ? "si" : "no") + "\n";
    mensaje += "Hora: " + config.hora + "\n";
    mensaje += "Destino: " + config.destino + "\n";
    mensaje += "Carpetas:\n" + carpetas_str;

    return mensaje;
}

std::string  estadoMonitor(const ConfigMonitor& config) {
    int cpu_entero = static_cast<int>(config.cpu);
    int ram_entero = static_cast<int>(config.ram);
    int disco_entero = static_cast<int>(config.disco);
    std::string mensaje = "=== Estado Monitor ===\n";
    mensaje += "Activo: " + std::string(config.activo ? "si" : "no") + "\n";
    mensaje += "Limite CPU: " + std::to_string(cpu_entero) + "%\n";
    mensaje += "Limite RAM: " + std::to_string(ram_entero) + "%\n";
    mensaje += "Limite Disco: " + std::to_string(disco_entero) + "%\n";

    return mensaje;
}

std::string  estadoOrganizador(const ConfigOrganizador& config) {
    std::string reglas_str;
    for (const auto& [extension, carpeta] : config.reglas) {
        reglas_str += extension + " -> " + carpeta + "\n";
    }

    std::string mensaje = "=== Estado Organizador ===\n";
    mensaje += "Activo: " + std::string(config.activo ? "si" : "no") + "\n";
    mensaje += "Carpeta vigilada: " + config.carpeta_vigilar + "\n";
    mensaje += "Reglas:\n" + reglas_str;

    return mensaje;
}

std::string generarDiagnostico(const ConfigSentinel& config) {
    return estadoBackup(config.backup) + "\n" + estadoMonitor(config.monitor) + "\n" + estadoOrganizador(config.organizador);
}