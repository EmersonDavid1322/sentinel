#include <vector>
#include <filesystem>
#include <string>
#include <chrono>
#include <thread>
#include "errores.h"
#include "logger.h"
#include "config.h"
#include "notificador.h"
#include "sentinel_estado.h"
namespace fs = std::filesystem;

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
        fs::create_directories(destino);
        logWarning("La carpeta destinataria no existe, se creo la carpeta destinataria del backup: " + destino);
    }
    return msg_carpetas;
}

void ejecutarBackup(const std::vector<std::string>& carpetas, const std::string& destino){
    for (const std::string& carpeta : carpetas){
        try{
            fs::path origen(carpeta);
            fs::path destino_final = fs::path(destino) / origen.filename();

            fs::copy(origen, destino_final, 
                    fs::copy_options::recursive | 
                    fs::copy_options::overwrite_existing);
        }
        catch(const fs::filesystem_error& e){
            logError("Error Backup: -" + std::string(e.what()));
            continue;
            }
    }
}

void hacerBackup(const std::vector<std::string>& carpetas, const std::string& destino, const std::string hora){
    try{
        time_t ahora = time(0);
        tm* tiempo = localtime(&ahora);
        char buffer[6];
        strftime(buffer, sizeof(buffer), "%H:%M", tiempo);
        std::string hora_actual = buffer;
        if (hora_actual != hora){
            return;
        }

        std::string carpetas_msg = verificarCarpetasBackup(carpetas, destino);
        ejecutarBackup(carpetas, destino);
        logInfo("Se realizo un bakup de forma correcta de las carpetas: " + carpetas_msg + " Destino: " + destino);
        enviarNotificación("Backup", "Se completo el bakup correctamente al la carpeta: " + destino, "INFO");

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

void loopBackup(const ConfigBackup& config){
    while (corriendo) {
        hacerBackup(config.carpetas, config.destino, config.hora);
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
}