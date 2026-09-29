#include "auxiliar_backup_local.h"
#include <filesystem>
namespace fs = std::filesystem;


void eliminarBackup(const fs::path& rutaBackup) {
    fs::remove_all(rutaBackup);
}
