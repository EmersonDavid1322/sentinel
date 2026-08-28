#include "comandos_backup.h"
#include "comandos_auxiliar.h"
#include "procesar_comandos.h"
#include "rutas.h"
#include "backup.h"
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

void cambiarForzarBackup(const std::string& accion) {
    std::filesystem::path ruta = obtenerRutaConfig();
    json datos = leerJSONActual(ruta);

    if (accion == "forzar") {
        datos["backup"]["forzar_backup"] = true;
    }else if (accion == "no_forzar") {
        datos["backup"]["forzar_backup"] = false;
    }else {
        enviarRespuesta("No existe la opción: " + accion + "\nDisponibles: 'forzar' y 'no_forzar'");
    }

    guardarJSON(datos, ruta);
    enviarRespuesta("Se a cambiado el parametro de forzar_backup");
}