#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <filesystem>
#include <sys/inotify.h>
#include <unistd.h>
#include "organizer.h"
#include "logger.h"
#include "errores.h"
#include "notificador.h"
namespace fs = std::filesystem;

std::vector<std::string> verificarCarpetas(const std::map<std::string, std::string>& carpetasRegla, const std::string& carpetaVigilar){
    std::vector<std::string> carpetas_fallidas;
    if (!fs::exists(carpetaVigilar)){
        throw ErrorOrganizador("Organizador: Carpeta a vigilar no existente: " + carpetaVigilar);
    }

    for (const auto& par : carpetasRegla){
        if (!fs::exists(par.second)){
            logWarning("Organizador: Carpeta no existende para archivo de extensión: " + par.first + " " + par.second + "\nSe cancelo la acción");
            carpetas_fallidas.push_back(par.second);
        }
    }
    return carpetas_fallidas;
}

void moverArchivo(const std::string& archivo, const std::map<std::string, std::string>& reglas, const std::vector<std::string>& carpetas_fallidas){
    fs::path ruta(archivo);
    std::string extension = ruta.extension().string();

    for (const auto& [extension_regla, destino] : reglas) {
    if (extension_regla != extension) continue;
    
    for (const std::string& carpeta : carpetas_fallidas) {
        if (carpeta == destino) {
            logWarning("Organizador: saltando archivo, carpeta destino no existe: " + destino);
            return;
        }
    }
    
    fs::path destino_final = fs::path(destino) / ruta.filename();
    fs::rename(ruta, destino_final);
    logInfo("Organizador: archivo movido: " + archivo + " -> " + destino);
    return;
    }
}

void ejecutarOrganizador(const std::map<std::string, std::string>& carpetasRegla, const std::string& carpetaVigilar){
    try{
        while (true){
            std::vector<std::string> carpetas_fallidas = verificarCarpetas(carpetasRegla, carpetaVigilar);
            int fd = inotify_init();
            int wd = inotify_add_watch(fd, carpetaVigilar.c_str(), IN_CREATE | IN_MOVED_TO);

            char buffer[4096];
            int bytes = read(fd, buffer, sizeof(buffer));
            if (bytes < 0) break;

            struct inotify_event* evento = (struct inotify_event*) buffer;

            if (evento->len > 0){
                std::string nombre_archivo = evento->name;

                std::string rutaCompleta = carpetaVigilar + "/" + nombre_archivo;

                moverArchivo(rutaCompleta, carpetasRegla, carpetas_fallidas);
            }
            inotify_rm_watch(fd, wd);
            close(fd);
        }
    }
    catch(const ErrorOrganizador& e){
        std::cout << "OrganizadorError: " << e.what() << std::endl;
        logError("Error en Organizador - " + std::string(e.what()));
        enviarNotificación("Error Organizador", "Ocurrio un error en el organizador: " + std::string(e.what()), "ERROR");
    }
    catch(const DaemonError& e){
        std::cout << "DaemonError: " << e.what() << std::endl;
        logError("Error en Deamon - " + std::string(e.what()));
        enviarNotificación("Error Deamon-Organizador", "Ocurrio un error en el organizador: " + std::string(e.what()), "ERROR");
    }
}
