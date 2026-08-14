#include "comandos_backup.h"
#include "comandos_auxiliar.h"
#include "procesar_comandos.h"
#include "rutas.h"
#include "backup.h"
#include "logger.h"
#include "notificador.h"
#include "errores.h"
#include "backup_auxiliar.h"
#include <filesystem>

//comandos backup_local
void agregarCarpetaBackup(const std::string& carpeta) {

    std::string carpeta_limpia = limpiarEspacios(carpeta);
    if (carpeta_limpia.empty()) {
        enviarRespuesta("No se permiten valores vacios en agregar carpeta");
        return;
    }

    std::filesystem::path ruta = obtenerRutaConfig();
    json datos = leerJSONActual(ruta);

    std::vector<std::string> carpetas = datos["backup"]["carpetas"];
    carpetas.push_back(carpeta_limpia);

    datos["backup"]["carpetas"] = carpetas;

    guardarJSON(datos, ruta);
    enviarRespuesta("Se a añadido la carpeta: " + carpeta_limpia);
}

void ejecutarBackupComando(const ConfigBackup& configBackup, const ConfigMonitor& configMonitor) {
    try {
        std::filesystem::path ruta = obtenerRutaConfig();
        json datos = leerJSONActual(ruta);

        ResultadoVerificacionRecursos resultado = verificarRecursosBackup(configBackup, configMonitor);

        if (resultado == ResultadoVerificacionRecursos::FORZADO) {
            logInfo("Continuando con el backup_local manual a pesar de recursos elevados (forzar_backup activo)", "sentinel.log");
        }
        else if (resultado == ResultadoVerificacionRecursos::CANCELADO_CPU) {
            logInfo("Se cancelo el backup_local 'Se regitro un uso elevado del cpu'", "sentinel.log");
            enviarRespuesta("Se cancelo el backup_local luego de varios intentos  'Se regitro un uso elevado del cpu'");
            return;
        }else if (resultado == ResultadoVerificacionRecursos::CANCELADO_DISCO) {
            logInfo("Se cancelo el backup_local 'Se regitro espacio elevado en el disco'", "sentinel.log");
            enviarRespuesta("Se cancelo el backup_local 'Se regitro espacio elevado en el disco'");
            return;
        }
        else {
            logInfo("Se inicio correctamente el backup_local", "sentinel.log");
        }


        std::string carpetas_msg = verificarCarpetasBackup(configBackup.carpetas, configBackup.destino);
        ejecutarBackup(configBackup.carpetas, configBackup.destino, configBackup.ignorar);

        logInfo("Se realizo un backup_local manual de las carpetas: " + carpetas_msg + " Destino: " + configBackup.destino, "sentinel.log");
        enviarRespuesta("Backup ejecutado correctamente. Carpetas: " + carpetas_msg + " Destino: " + configBackup.destino);
        enviarNotificación("Backup", "Se realizao el backup_local manual de forma exitosa", "INFO");
    }
    catch (const ErrorBackup& e) {
        enviarRespuesta("Error al ejecutar backup_local: " + std::string(e.what()));
        enviarNotificación("Backup", "Hubo un error durante la ejecutcion del backup_local: " + std::string(e.what()), "ERROR");
    }
    catch (const DaemonError& e) {
        enviarRespuesta("Error Deamon al ejecutar backup_local: " + std::string(e.what()));
        enviarNotificación("Backup", "Hubo un error durante la ejecutcion del backup_local: " + std::string(e.what()), "ERROR");
    }
}
