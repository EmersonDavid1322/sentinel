#include "backup_auxiliar.h"
#include <filesystem>
#include <string>
#include <vector>
namespace fs = std::filesystem;

bool debeIgnorarce(const fs::path& ruta, const std::vector<std::string>& lista_ignorar) {
    for (const auto& parte : ruta) {
        for (const auto& regla : lista_ignorar) {
            if (parte == regla || parte.extension() == regla) {
                return true;
            }
        }
    }
    return false;
}
