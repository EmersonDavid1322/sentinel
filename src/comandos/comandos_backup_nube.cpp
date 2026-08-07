#include "comandos_backup_nube.h"
#include "backup_nube_listado.h"
#include "logger.h"
#include "errores.h"
#include "procesar_comandos.h"
#include <vector>

void mostrarListadoComando(const ConfigBackupNube& config) {
    try {
        std::vector<ArchivoRemoto> listado = listaArchivosRemotos(config);
        std::string mensaje;

        for (const auto& archivo : listado) {
            mensaje += "\nNombre: " + archivo.nombre + "\n" +
                "Ruta: " + archivo.ruta + "\n" +
                "Carpeta: " + std::to_string(archivo.esCarpeta ) + "\n";
        }
        enviarRespuesta(mensaje);
    }
    catch (const ErrorBackupAPI& e) {
        logError("Ocurrio un error con el API del backup a la nube al intentar conseguir el listado de archivos remotos: "
            + std::string(e.what()), "sentinel.log");
        enviarRespuesta("Ocurrio un error con el API del backup a la nube al intentar conseguir el listado de archivos remotos: "
            + std::string(e.what()));
    }
    catch (const ErrorBackupRED& e) {
        logError("Ocurrio un error con la peticion al servidor al intentar conseguir el listado de archivos remotos: "
            + std::string(e.what()),"sentinel.log");
        enviarRespuesta("Ocurrio un error con la peticion al servidor al intentar conseguir el listado de archivos remotos: "
            + std::string(e.what()));
    }
}
