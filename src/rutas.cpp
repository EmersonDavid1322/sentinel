#include <filesystem>
namespace fs = std::filesystem;

std::filesystem::path obtenerRutaBase(){
    return  fs::canonical("/proc/self/exe").parent_path();
}