#include <vector>
#include <filesystem>
#include <string>
#include <chrono>
#include "errores.h"
#include "backup.h"
#include "auxiliar_backup_local.h"
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
    fs::path carpeta_backup;
    bool hubo_errores = false;

    if (configBackup.crear_carpeta_backup) {
        logInfo("Se creara la carpeta para backup", "backups.log");
    }

    for (const std::string& carpeta : configBackup.carpetas){

        fs::path origen(carpeta);
        if (configBackup.crear_carpeta_backup) {
            carpeta_backup = destino / nombre_carpeta / origen.filename();
        }else {
            carpeta_backup = destino  / origen.filename();
        }
        fs::create_directories(carpeta_backup);

        try {
            for (auto it = fs::recursive_directory_iterator(origen); it != fs::recursive_directory_iterator(); ++it) {
                const auto& entrada = *it;
                try {
                    if (debeSubirseArchivo(entrada, configBackup.ignorar, configBackup.solo_modificados_hoy) == FiltroArchivos::IGNORAR_CARPETA) {
                        it.disable_recursion_pending();
                        logInfo("Se ignoro la carpeta completa: " + entrada.path().string(), "backups.log");
                        continue;
                    }

                    if (debeSubirseArchivo(entrada, configBackup.ignorar, configBackup.solo_modificados_hoy) == FiltroArchivos::IGNORAR) {
                        logInfo("Se ignoro un archivo que no paso los filtros: " + entrada.path().string(), "backups.log");
                        continue;
                    }

                    if (!fs::is_directory(entrada.path())) {
                        std::uintmax_t tamaño_archivo = fs::file_size(entrada.path());
                        fs::space_info informe_espacio = fs::space(configBackup.destino);

                        if (informe_espacio.available < tamaño_archivo) {
                            logError("No hay suficiente espacio en el destino para el archivo: " + entrada.path().string(), "backups.log");
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
                catch(const fs::filesystem_error& e){
                    hubo_errores = true;
                    enviarNotificación("Backup", "Error Backup: -" + std::string(e.what()), "WARNING");
                    logError("Error Backup: -" + std::string(e.what()), "backups.log");
                }
            }

            if (configBackup.eliminar_ultimo_backup_registrado) {
                if (!hubo_errores) {
                    if (!fs::exists(extraerRutaUltimoBackup("backup"))) {
                        logWarning("Anterior backups no existente en la dirrecion registrada: " + extraerRutaUltimoBackup("backup").string(), "backups.log");
                    }else {
                        eliminarAnteriorBackup();
                        logInfo("Se elimino correctamente el ultimo backup registrado: " + extraerRutaUltimoBackup("backup").string(), "backups.log");
                    }
                }else {
                    logInfo("No se eliminara el ultimo backup ya que hubieron errores en el backup actual","backups.log");
                }
            }

            if (configBackup.crear_carpeta_backup) {
                if (!hubo_errores) {
                    guardarRutaUltimoBackup("backup", carpeta_backup.parent_path().string());
                    logInfo("Se guardo correctamente la ruta del backup: " + carpeta_backup.parent_path().string(), "backups.log");
                }else {
                    logInfo("No se guardara el actual backup ya que hubieron errores","backups.log");
                }
            }

        }catch(const fs::filesystem_error& e){
            enviarNotificación("Backup", "Error Backup: -" + std::string(e.what()), "WARNING");
            logError("Error Backup filesystem: -" + std::string(e.what()), "backups.log");
        }
        catch (const ErrorBackup& e) {
            enviarNotificación("Backup", "Error Backup: -" + std::string(e.what()), "WARNING");
            logError("Error Backup: -" + std::string(e.what()), "backups.log");
        }
    }
    logInfo("Se a completado el backup local", "sentinel.log");
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

        logInfo("Se realizo un bakup revise 'backups.log': " + carpetas_msg + " Destino: " + config_backup.destino, "sentinel.log");
        enviarNotificación("Backup", "Se completo el bakup revise 'backups.log': " + config_backup.destino, "INFO");

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