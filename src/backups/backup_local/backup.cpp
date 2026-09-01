#include <vector>
#include <filesystem>
#include <string>
#include <chrono>
#include "errores.h"
#include "backup.h"
#include "logger.h"
#include "config.h"
#include "notificador.h"
#include "sentinel_estado.h"
#include "config_compartida.h"
#include "monitor.h"
#include "auxiliar_compartido.h"
namespace fs = std::filesystem;

ResultadoVerificacionRecursos verificarRecursosBackup(const ConfigBackup& configBackup, const ConfigMonitor& configMonitor) {
    bool disco_superado = uso_disco() >= configMonitor.disco;
    bool cpu_superado = uso_cpu() >= configMonitor.cpu;

    if ((disco_superado || cpu_superado) && configBackup.forzar_backup) {
        return ResultadoVerificacionRecursos::FORZADO;
    }

    if (disco_superado) {
        return ResultadoVerificacionRecursos::CANCELADO_DISCO;
    }

    if (cpu_superado) {
        return ResultadoVerificacionRecursos::CANCELADO_CPU;
    }
    return ResultadoVerificacionRecursos::OK;
}

std::string verificarCarpetasBackup(const std::vector<std::string>& carpetas, const std::string& destino){
    std::string msg_carpetas;

    for (const std::string& carpeta : carpetas){
        if (!fs::exists(carpeta)){
            throw ErrorBackup("La carpeta no existe: " + carpeta);
        }

        msg_carpetas += " " + carpeta;

    }
    if (!fs::exists(destino)){
        try {
            fs::create_directories(destino);
            logWarning("La carpeta destinataria no existe, se creo la carpeta destinataria del backup_local: " + destino, "sentinel.log");
        }
        catch (const std::filesystem::filesystem_error& e) {
            throw ErrorBackup("No se pudo crear la carpeta destino '" + destino + "' "
                                "posible ubicacion erronea: " + std::string(e.what()));
        }
    }
    return msg_carpetas;
}

void ejecutarBackup(const ConfigBackup& configBackup){
    std::string nombre_carpeta = obtenerNombreCarpetaBackup();
    fs::path destino(configBackup.destino);

    for (const std::string& carpeta : configBackup.carpetas){
        try{
            fs::path origen(carpeta);
            fs::path carpeta_backup = destino / nombre_carpeta / origen.filename();
            fs::create_directories(carpeta_backup);
            for (auto it = fs::recursive_directory_iterator(origen); it != fs::recursive_directory_iterator(); ++it) {
                const auto& entrada = *it;

                if (fs::is_directory(entrada) && debeIgnorarce(entrada.path(), configBackup.ignorar)) {
                    logWarning("Se ignoro la carpeta completa: " + entrada.path().string(), "backups.log");
                    it.disable_recursion_pending();
                    continue;
                }

                if (debeIgnorarce(entrada.path(), configBackup.ignorar)) {
                    logWarning("Se ignoro un archivo :" + entrada.path().string(), "backups.log");
                    continue;
                }

                if (!fs::is_regular_file(entrada) && !fs::is_directory(entrada)) {
                    logWarning("Backup: se omitió un archivo de tipo especial (no regular ni carpeta): " + entrada.path().string(), "backups.log");
                    continue;
                }

                if (configBackup.solo_modificados_hoy) {
                    if (!archivoModificadoCreadoHoy(entrada.path())) {
                        logWarning("Backup: se omitió un archivo que no se modificó/creó hoy: " + entrada.path().string(), "backups.log");
                        continue;
                    }
                }

                fs::path destino_final = carpeta_backup / fs::relative(entrada.path(), origen);

                fs::path carpetaDestinoArchvo = destino_final.parent_path();

                if (!fs::exists((carpetaDestinoArchvo))) {
                    fs::create_directories(carpetaDestinoArchvo);
                }

                if (fs::is_directory(entrada)) {
                    fs::create_directories(destino_final);
                    logInfo("Se creo correctamente la carpeta " + entrada.path().string() , "backups.log");
                } else {
                    fs::copy_file(entrada.path(), destino_final, fs::copy_options::overwrite_existing);
                    logInfo("Se copio correctamente el archivo " + entrada.path().string() , "backups.log");
                }
            }
        }
        catch(const fs::filesystem_error& e){
            enviarNotificación("Backup", "Error Backup: -" + std::string(e.what()), "WARNING");
            logError("Error Backup: -" + std::string(e.what()), "backups.log");
            continue;
            }
    }
    logInfo("Se a completado el backup_local local de forma exitosa", "sentinel.log");
}

void hacerBackup(const ConfigBackup& config_backup, const ConfigMonitor& config_monitor){
    try{
        ResultadoVerificacionRecursos resultado = verificarRecursosBackup(config_backup, config_monitor);

        if (resultado == ResultadoVerificacionRecursos::FORZADO) {
            logInfo("Continuando con el backup_local a pesar de recursos elevados (forzar_backup activo)", "sentinel.log");
            enviarNotificación("Backup", "Continuando con el backup_local a pesar de recursos elevados (forzar_backup activo)", "WARNING");
        }
        else if (resultado == ResultadoVerificacionRecursos::CANCELADO_CPU) {
            logInfo("Se cancelo el backup_local 'Se regitro un uso elevado del cpu'", "sentinel.log");
            enviarNotificación("Backup","Se cancelo el backup_local luego de varios intentos  'Se regitro un uso elevado del cpu'", "WARNING");
            return;
        }else if (resultado == ResultadoVerificacionRecursos::CANCELADO_DISCO) {
            logInfo("Se cancelo el backup_local 'Se regitro espacio elevado en el disco'", "sentinel.log");
            enviarNotificación("Backup","Se cancelo el backup_local 'Se regitro espacio elevado en el disco'", "WARNING");
            return;
        }
        else {
            logInfo("Se inicio correctamente el backup_local", "sentinel.log");
        }

        limpiarLog();
        std::string carpetas_msg = verificarCarpetasBackup(config_backup.carpetas, config_backup.destino);
        ejecutarBackup(config_backup);

        logInfo("Se realizo un bakup de forma correcta de las carpetas: " + carpetas_msg + " Destino: " + config_backup.destino, "sentinel.log");
        enviarNotificación("Backup", "Se completo el bakup correctamente a la carpeta: " + config_backup.destino, "INFO");

    }
    catch(const ErrorBackup& e){
        logError("Error en backup_local - " + std::string(e.what()), "sentinel.log");
        enviarNotificación("Error backup_local", "Ocurrio un error en el intento de bakup: " + std::string(e.what()), "ERROR");
    }

    catch(const DaemonError& e){
        logError("Error en backup_local - " + std::string(e.what()), "sentinel.log");
        enviarNotificación("Error Deamon-backup_local", "Ocurrio un error en el intento de bakup: " + std::string(e.what()), "ERROR");
    }
}

void loopBackup(ConfigCompartida& config_compartida){
    while (corriendo) {
        ConfigSentinel config = config_compartida.obtener();

        if (config.backup.activo) {
            if (verificarHoraBackup(config.backup.hora)) {
                hacerBackup(config.backup, config.monitor);
            }
        }

        std::unique_lock<std::mutex> lock(mtx_apagado);
        cv_apagado.wait_for(lock, std::chrono::seconds(60), [] { return !corriendo.load(); });
    }
}
