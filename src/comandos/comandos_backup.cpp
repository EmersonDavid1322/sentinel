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

std::string mensajeResultadoBackup(const ResultadoHacerBackup& resultado) {
    std::string mensaje;
    if (resultado.resultado_backup == ResultadoBackupEnum::EXITO) {
        mensaje = "El backup se a realizado con existo con la configuración establecida";
    }else if (resultado.resultado_backup == ResultadoBackupEnum::FALLIDO) {
        if (resultado.causa.has_value()) {
            if (resultado.causa == CausaBackup::CONFIG_INVALIDA) {
                mensaje = "EL backup a fallado debido a que la configuración actual es invalida";
            }else if (resultado.causa == CausaBackup::CARPETA_NO_EXISTE) {
                mensaje = "El backup a fallado debido a que hay una carpeta inexistente en la lista de carpetas a copiar";
            }else if (resultado.causa == CausaBackup::DESTINO_INACCESIBLE) {
                mensaje = "El backup a fallado debido a que no se puedo acceder al destino actual";
            }else if (resultado.causa == CausaBackup::DESCONOCIDA) {
                mensaje = "El backup a fallado debido a una causa desconocida";
            }
        }else {
            mensaje = "El backup a dado resultado fallido y no se a encontrado una causa registrada";
        }
    }else if (resultado.resultado_backup == ResultadoBackupEnum::COMPLETADO_CON_ERRORES) {
        mensaje = "El backup se a finalizado con errores, revise los logs para información detallada";
    }else if (resultado.resultado_backup == ResultadoBackupEnum::CANCELADO_CPU) {
        mensaje = "El backup a sido cancelado debido a que se detecto un alto uso de cpu (Active ´forzar_backup' si desea que no se cancele el backup)";
    }else if (resultado.resultado_backup == ResultadoBackupEnum::CANCELADO_DISCO) {
        mensaje = "El backup a sido cancelado debido a que se detecto que se sobrepasa el limite de espacio en el dispositivo (Active ´forzar_backup' si desea que no se cancele el backup)";
    }
    return mensaje;
}