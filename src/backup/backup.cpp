#include <vector>
#include <filesystem>
#include <string>
#include <chrono>
#include <thread>
#include "errores.h"
#include "backup.h"
#include "logger.h"
#include "config.h"
#include "notificador.h"
#include "sentinel_estado.h"
#include "config_compartida.h"
#include "monitor.h"
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
        else{
            msg_carpetas += " " + carpeta;
        }
    }
    if (!fs::exists(destino)){
        try {
            fs::create_directories(destino);
            logWarning("La carpeta destinataria no existe, se creo la carpeta destinataria del backup: " + destino);
        }
        catch (const std::filesystem::filesystem_error& e) {
            throw ErrorBackup("No se pudo crear la carpeta destino '" + destino + "' "
                                "posible ubicacion erronea: " + std::string(e.what()));
        }
    }
    return msg_carpetas;
}

void ejecutarBackup(const std::vector<std::string>& carpetas, const std::string& destino){
    for (const std::string& carpeta : carpetas){
        try{
            fs::path origen(carpeta);
            fs::path carpeta_backup = destino / origen.filename();
            fs::create_directories(carpeta_backup);
            for (const auto& entrada : fs::recursive_directory_iterator(origen)) {
                if (!fs::is_regular_file(entrada) && !fs::is_directory(entrada)) {
                    logWarning("Backup: se omitió un archivo de tipo especial (no regular ni carpeta): " + entrada.path().string());
                    continue;
                }

                fs::path destino_final = carpeta_backup / fs::relative(entrada.path(), origen);

                if (fs::is_directory(entrada)) {
                    fs::create_directories(destino_final);
                } else {
                    fs::copy_file(entrada.path(), destino_final, fs::copy_options::overwrite_existing);
                }
            }
        }
        catch(const fs::filesystem_error& e){
            enviarNotificación("Backup", "Error Backup: -" + std::string(e.what()), "WARNING");
            logError("Error Backup: -" + std::string(e.what()));
            continue;
            }
    }
}

void hacerBackup(const ConfigBackup& config_backup, const ConfigMonitor& config_monitor){
    try{
        time_t ahora = time(0);
        tm* tiempo = localtime(&ahora);
        char buffer[6];
        strftime(buffer, sizeof(buffer), "%H:%M", tiempo);
        std::string hora_actual = buffer;
        if (hora_actual != config_backup.hora){
            return;
        }

        ResultadoVerificacionRecursos resultado = verificarRecursosBackup(config_backup, config_monitor);

        if (resultado == ResultadoVerificacionRecursos::FORZADO) {
            logInfo("Continuando con el backup a pesar de recursos elevados (forzar_backup activo)");
            enviarNotificación("Backup", "Continuando con el backup a pesar de recursos elevados (forzar_backup activo)", "WARNING");
        }
        else if (resultado == ResultadoVerificacionRecursos::CANCELADO_CPU) {
            logInfo("Se cancelo el backup 'Se regitro un uso elevado del cpu'");
            enviarNotificación("Backup","Se cancelo el backup luego de varios intentos  'Se regitro un uso elevado del cpu'", "WARNING");
            return;
        }else if (resultado == ResultadoVerificacionRecursos::CANCELADO_DISCO) {
            logInfo("Se cancelo el backup 'Se regitro espacio elevado en el disco'");
            enviarNotificación("Backup","Se cancelo el backup 'Se regitro espacio elevado en el disco'", "WARNING");
            return;
        }
        else {
            logInfo("Se inicio correctamente el backup a las: " + hora_actual);
        }

        std::string carpetas_msg = verificarCarpetasBackup(config_backup.carpetas, config_backup.destino);
        ejecutarBackup(config_backup.carpetas, config_backup.destino);
        logInfo("Se realizo un bakup de forma correcta de las carpetas: " + carpetas_msg + " Destino: " + config_backup.destino);
        enviarNotificación("Backup", "Se completo el bakup correctamente a la carpeta: " + config_backup.destino, "INFO");

    }
    catch(const ErrorBackup& e){
        logError("Error en backup - " + std::string(e.what()));
        enviarNotificación("Error backup", "Ocurrio un error en el intento de bakup: " + std::string(e.what()), "ERROR");
    }

    catch(const DaemonError& e){
        logError("Error en backup - " + std::string(e.what()));
        enviarNotificación("Error Deamon-backup", "Ocurrio un error en el intento de bakup: " + std::string(e.what()), "ERROR");
    }
}

void loopBackup(ConfigCompartida& config_compartida){
    while (corriendo) {
        ConfigSentinel config = config_compartida.obtener();

        if (config.backup.activo) {
            hacerBackup(config.backup, config.monitor);
        }

        std::unique_lock<std::mutex> lock(mtx_apagado);
        cv_apagado.wait_for(lock, std::chrono::seconds(60), [] { return !corriendo.load(); });
    }
}