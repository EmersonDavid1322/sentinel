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

void validarConfiguracionBackup(const ConfigBackup& configBackup) {
    if (configBackup.carpetas.empty()) {
        throw ErrorBackup("No se configuraron carpetas para el backup local");
    }

    if (configBackup.destino.empty()) {
        throw ErrorBackup("No se configuró una carpeta destino para el backup local");
    }

    if (configBackup.eliminar_ultimo_backup_registrado && !configBackup.crear_carpeta_backup) {
        throw ErrorBackup(
            "La opción eliminar_ultimo_backup_registrado requiere crear_carpeta_backup activada");
    }
}

ResultadoCopiaBackup copiarCarpetasBackup(const ConfigBackup& configBackup){
    std::string nombre_carpeta = obtenerNombreCarpetaBackup();
    fs::path destino(configBackup.destino);
    fs::path ruta_backup = configBackup.crear_carpeta_backup ? destino / nombre_carpeta : destino;
    bool hubo_errores = false;

    if (configBackup.crear_carpeta_backup) {
        logInfo("Se creara la carpeta para backup", "backups.log");
    }

    for (const std::string& carpeta : configBackup.carpetas){

        fs::path origen(carpeta);
        fs::path carpeta_backup = ruta_backup / origen.filename();

        try {
            fs::create_directories(carpeta_backup);
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
                            hubo_errores = true;
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

        }catch(const fs::filesystem_error& e){
            hubo_errores = true;
            enviarNotificación("Backup", "Error Backup: -" + std::string(e.what()), "WARNING");
            logError("Error Backup filesystem: -" + std::string(e.what()), "backups.log");
        }
        catch (const ErrorBackup& e) {
            hubo_errores = true;
            enviarNotificación("Backup", "Error Backup: -" + std::string(e.what()), "WARNING");
            logError("Error Backup: -" + std::string(e.what()), "backups.log");
        }
    }

    return { !hubo_errores, ruta_backup };
}

bool ejecutarBackup(const ConfigBackup& configBackup){
    ResultadoCopiaBackup resultado = copiarCarpetasBackup(configBackup);

    if (!resultado.completado) {
        logWarning("El backup local terminó con errores. Se conserva el backup anterior y la copia parcial no se registrará.", "backups.log");
        logWarning("El backup local terminó con errores. Revise backups.log.", "sentinel.log");
        enviarNotificación("Backup", "El backup terminó con errores; se conservó el anterior y la copia parcial no se registró.", "WARNING");
        return false;
    }

    fs::path ultimo_backup;
    if (configBackup.eliminar_ultimo_backup_registrado) {
        ultimo_backup = extraerRutaUltimoBackup("backup");
    }

    if (configBackup.crear_carpeta_backup) {
        guardarRutaUltimoBackup("backup", resultado.ruta_backup.string());
        logInfo("Se guardó correctamente la ruta del backup: " + resultado.ruta_backup.string(), "backups.log");
    }

    if (configBackup.eliminar_ultimo_backup_registrado) {
        if (ultimo_backup.empty()) {
            logInfo("No había un backup anterior registrado para eliminar.", "backups.log");
        } else if (ultimo_backup.lexically_normal() == resultado.ruta_backup.lexically_normal()) {
            logWarning("El backup anterior coincide con el actual; no se eliminará.", "backups.log");
        } else if (!fs::exists(ultimo_backup)) {
            logWarning("El backup anterior registrado no existe: " + ultimo_backup.string(), "backups.log");
        } else {
            eliminarBackup(ultimo_backup);
            logInfo("Se eliminó correctamente el backup anterior: " + ultimo_backup.string(), "backups.log");
        }
    }

    logInfo("Se completó el backup local", "sentinel.log");
    return true;
}

void hacerBackup(const ConfigBackup& config_backup, const ConfigMonitor& config_monitor){
    try{
        validarConfiguracionBackup(config_backup);
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
        if (!ejecutarBackup(config_backup)) {
            return;
        }

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