#include <filesystem>
namespace fs = std::filesystem;

void asegurarCarpetasSentinel() {
    fs::create_directories("/etc/sentinel");
    fs::create_directories("/var/log/sentinel");
    fs::create_directories("/var/lib/sentinel");
}

std::filesystem::path obtenerRutaConfig() {
    return "/etc/sentinel/sentinel.json";
}

std::filesystem::path obtenerRutaLogs() {
    return "/var/log/sentinel";
}

std::filesystem::path obtenerRutaEstado() {
    return "/var/lib/sentinel/";
}