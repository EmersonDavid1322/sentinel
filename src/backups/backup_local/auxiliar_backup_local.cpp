#include "auxiliar_backup_local.h"
#include "errores.h"
#include "auxiliar_compartido.h"
#include <filesystem>
namespace fs = std::filesystem;


void eliminarBackup(const fs::path& rutaBackup) {
    fs::remove_all(rutaBackup);
}
