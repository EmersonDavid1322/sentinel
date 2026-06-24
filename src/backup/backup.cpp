#include <iostream>
#include <vector>
#include <filesystem>
#include <string>
#include <ctime>
#include "errores.h"
#include "rutas.h"
namespace fs = std::filesystem;
fs::path ruta_base = obtenerRutaBase();


void verificarCarpetas(const std::vector<std::string>& carpetas, const std::string& destino){
    for (const std::string carpeta : carpetas){
        if (!fs::exists(carpeta)){
            throw ErrorBackup("La carpeta no existe: " + carpeta);
        }
    }
    if (!fs::exists(destino)){
        fs::create_directories(destino);
    }
}

void ejecutarBackup(const std::vector<std::string>& carpetas, const std::string& destino){
    for (const std::string& carpeta : carpetas){
        fs::copy(carpeta, destino, fs::copy_options::recursive);
    }
}



void hacerBackup(const std::vector<std::string>& carpetas, const std::string& destino, const std::string hora){
    try{
        verificarCarpetas(carpetas, destino);
        ejecutarBackup(carpetas, destino);

        
    }
    catch(const ErrorBackup& e){
        std::cout << "Error Backup: " << e.what() << std::endl;
    }

    catch(const DaemonError& e){
        std::cout << "DeamonError: " << e.what() << std::endl;
    }
}