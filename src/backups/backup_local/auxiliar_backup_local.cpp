#include "auxiliar_backup_local.h"
#include "errores.h"
#include "auxiliar_compartido.h"
#include <filesystem>
#include "logger.h"
namespace fs = std::filesystem;


void eliminarAnteriorBackup() {
    fs::path ultimo_backup = extraerRutaUltimoBackup("backup");

    if (!fs::exists(ultimo_backup)) {
        logInfo("Error anterior backups no existente en la dirrecion registrada: " + ultimo_backup.string(), "backups.log");
    }

    fs::remove_all(ultimo_backup);
}