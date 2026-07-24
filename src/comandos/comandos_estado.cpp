#include "comandos_estado.h"
#include "config_loader.h"
#include <string>

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