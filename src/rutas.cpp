#include <filesystem>
#include "errores.h"
namespace fs = std::filesystem;

std::filesystem::path obtenerRutaBase(){
    try {
        return fs::canonical("/proc/self/exe").parent_path();
    } catch (const std::filesystem::filesystem_error& e) {
        throw ErrorConfig("No se pudo determinar la ruta base del ejecutable: " + std::string(e.what()));
    }
}

void asegurarCarpetasSentinel() {
    fs::create_directories("/etc/sentinel");
    fs::create_directories("/var/log/sentinel");
    fs::create_directories("/var/lib/sentinel");
}

std::filesystem::path obtenerRutaConfig() {
    return "/home/Emerson/proyectos_personales/c++/proyectos/sentinel/config/sentinel.json";
}

std::filesystem::path obtenerRutaLogs() {
    return "/home/Emerson/proyectos_personales/c++/proyectos/sentinel/logs/";
}

std::filesystem::path obtenerRutaEstado() {
    return "/home/Emerson/proyectos_personales/c++/proyectos/sentinel/config/";
}