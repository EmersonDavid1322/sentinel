#include <iostream>
#include <vector>
#include <filesystem>
#include <string>
#include <ctime>
#include <fstream>
#include "errores.h"
#include "rutas.h"
namespace fs = std::filesystem;

void registroResultado(const std::string& resultado){
    fs::path ruta_base = obtenerRutaBase();
    fs::path ruta_log = ruta_base / "logs" / "sentinel.log";

    time_t ahora = time(0);
    std::string fecha = ctime(&ahora);

    std::ofstream log(ruta_log, std::ios::app);
    log << "[" << fecha.substr(0, fecha.size()-1) << "] " << resultado << std::endl;
}

std::string verificarCarpetas(const std::vector<std::string>& carpetas, const std::string& destino){
    std::string msg_carpetas;

    for (const std::string carpeta : carpetas){
        if (!fs::exists(carpeta)){
            throw ErrorBackup("La carpeta no existe: " + carpeta);
        }
        else{
            msg_carpetas += " " + carpeta;
        }
    }
    if (!fs::exists(destino)){
        fs::create_directories(destino);
        registroResultado("Se creo la carpeta destinataria del backup: " + destino);
    }
    return msg_carpetas;
}

void ejecutarBackup(const std::vector<std::string>& carpetas, const std::string& destino){
    for (const std::string& carpeta : carpetas){
        fs::path origen(carpeta);
        fs::path destino_final = fs::path(destino) / origen.filename();

        fs::copy(origen, destino_final, 
                fs::copy_options::recursive | 
                fs::copy_options::overwrite_existing);
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

        std::string carpetas_msg = verificarCarpetas(carpetas, destino);
        ejecutarBackup(carpetas, destino);
        registroResultado("Se realizo un bakup de forma correcta de las carpetas: " + carpetas_msg + " Destino: " + destino);

    }
    catch(const ErrorBackup& e){
        std::cout << "Error Backup: " << e.what() << std::endl;
        registroResultado("Error en backup - " + std::string(e.what()));
    }

    catch(const DaemonError& e){
        std::cout << "DeamonError: " << e.what() << std::endl;
        registroResultado("Error en backup - " + std::string(e.what()));
    }
}