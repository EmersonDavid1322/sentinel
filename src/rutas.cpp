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