#include "loop_backup_nube.h"
#include "sentinel_estado.h"
#include "backup_nube_subida.h"
#include "backup_nube_bajada.h"
#include "config_compartida.h"

void loopBackupNube(ConfigCompartida& config_compartida) {
    while (corriendo) {
        ConfigSentinel config = config_compartida.obtener();

        if (config.backup_nube.activo) {
            ejecutarBackupNube(config.backup_nube);
            ejecutarBajadaArchivosNube(config.backup_nube);
        }
        std::unique_lock<std::mutex> lock(mtx_apagado);
        cv_apagado.wait_for(lock, std::chrono::seconds(60), [] { return !corriendo.load(); });
    }
}