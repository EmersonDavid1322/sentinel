#include <filesystem>
namespace fs = std::filesystem;

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