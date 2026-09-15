#include "auxiliar_backup_local.h"
#include "errores.h"
#include "auxiliar_compartido.h"
#include <filesystem>
namespace fs = std::filesystem;


void eliminarAnteriorBackup() {
    fs::path ultimo_backup = extraerRutaUltimoBackup("backup");

    fs::remove_all(ultimo_backup);
}