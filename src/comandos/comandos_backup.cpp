#include "comandos_backup.h"
#include "comandos_auxiliar.h"
#include "procesar_comandos.h"
#include "rutas.h"
#include "backup.h"
#include "logger.h"
#include "notificador.h"
#include "errores.h"
#include <filesystem>

//comandos backup
void agregarCarpetaBackup(const std::string& carpeta) {

    std::string carpeta_limpia = limpiarEspacios(carpeta);
    if (carpeta.empty()) {
        enviarRespuesta("No se permiten valores vacios en agregar carpeta");
        return;
    }

    std::filesystem::path ruta = obtenerRutaBase() / "config" / "sentinel.json";
    json datos = leerJSONActual(ruta);

    std::vector<std::string> carpetas = datos["backup"]["carpetas"];
    carpetas.push_back(carpeta_limpia);

    datos["backup"]["carpetas"] = carpetas;

    guardarJSON(datos, ruta);
    enviarRespuesta("Se a añadido la carpeta: " + carpeta_limpia);
}

void ejecutarBackupComando(const ConfigBackup& configBackup) {
    try {
        std::filesystem::path ruta = obtenerRutaBase() / "config" / "sentinel.json";
        json datos = leerJSONActual(ruta);

        enviarRespuesta("Se inicio el backup manual");
        std::string carpetas_msg = verificarCarpetasBackup(configBackup.carpetas, configBackup.destino);
        ejecutarBackup(configBackup.carpetas, configBackup.destino);

        logInfo("Se realizo un backup manual de las carpetas: " + carpetas_msg + " Destino: " + configBackup.destino);
        enviarRespuesta("Backup ejecutado correctamente. Carpetas: " + carpetas_msg + " Destino: " + configBackup.destino);
        enviarNotificación("Backup", "Se realizao el backup manual de forma exitosa", "INFO");
    }
    catch (const ErrorBackup& e) {
        enviarRespuesta("Error al ejecutar backup: " + std::string(e.what()));
        enviarNotificación("Backup", "Hubo un error durante la ejecutcion del backup " + std::string(e.what()), "ERROR");
    }
    catch (const DaemonError& e) {
        enviarRespuesta("Error Deamon al ejecutar backup: " + std::string(e.what()));
        enviarNotificación("Backup", "Hubo un error durante la ejecutcion del backup " + std::string(e.what()), "ERROR");
    }
}
