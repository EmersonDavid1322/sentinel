#include "comandos_backup_nube.h"
#include "backup_nube_listado.h"
#include "backup_nube_subida.h"
#include "logger.h"
#include "errores.h"
#include "procesar_comandos.h"
#include "backup_nube_auxiliar.h"
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
        if (e.codigoHTTP == 401) {
            std::string token_nuvo = renovarAccessToken(config);
            actualizarToken(token_nuvo);
            logInfo("Se a actualizado el token", "sentinel.log");
            enviarRespuesta("Se a actualizado el token, vuelva a intentarlo");
        }
        else {
            logError("Ocurrio un error con el API del backup a la nube al intentar conseguir el listado de archivos remotos: "
            + std::string(e.what()), "sentinel.log");
            enviarRespuesta("Ocurrio un error con el API del backup a la nube al intentar conseguir el listado de archivos remotos: "
                + std::string(e.what()));
        }
    }
    catch (const ErrorBackupRED& e) {
        logError("Ocurrio un error con la peticion al servidor al intentar conseguir el listado de archivos remotos: "
            + std::string(e.what()),"sentinel.log");
        enviarRespuesta("Ocurrio un error con la peticion al servidor al intentar conseguir el listado de archivos remotos: "
            + std::string(e.what()));
    }
}

void ejecutarBackupNubeComando(const ConfigBackupNube& config) {
    enviarRespuesta("Backup a la nube iniciado correctametne, por favor revise los logs para información detallada");

    logInfo("Se incio el backup a la nube por comando", "sentinel.log");
    limpiarLog();
    std::string token = config.token;

    namespace fs = std::filesystem;
    for (const auto& carpeta : config.carpetas) {
        fs::path origen(carpeta);
        for (auto it = fs::recursive_directory_iterator(origen); it != fs::recursive_directory_iterator(); ++it) {
            const auto& entrada = *it;

            fs::path archivo = origen / entrada;

            if (fs::is_directory(entrada) && debeIgnorarce(entrada.path(), config.ignorar)) {
                logWarning("Se ignoro la carpeta completa: " + entrada.path().string(), "backups.log");
                it.disable_recursion_pending();
                continue;
            }

            if (debeIgnorarce(entrada.path(), config.ignorar)) {
                logWarning("Se ignoro un archivo: " + entrada.path().string(), "backups.log");
                continue;
            }

            if (!fs::is_regular_file(entrada)) {
                logWarning("Se ignoro un archivo de tipo no regular: " + archivo.string(), "backups.log");
                continue;
            }

            fs::path ruta_relativa = fs::relative(entrada.path(), origen);
            std::string ruta_remota = config.carpeta_remota + "/" + ruta_relativa.string();

            try{
                logInfo("Se incio la subida del archivo: " + archivo.string(), "backups.log");
                subirArchivoStreaming(archivo.string(), ruta_remota, token);
            }
            catch (const std::filesystem::filesystem_error& e) {
                logError("Ocurrio un error con el manejo de archivos loca: " + std::string(e.what()), "sentinel.log");
            }
            catch (const ErrorBackupAPI& e) {
                if (e.codigoHTTP == 401) {
                    token = renovarAccessToken(config);
                    actualizarToken(token);
                    logInfo("Se a actualizado el token", "sentinel.log");
                    subirArchivoStreaming(archivo.string(),ruta_remota, token);
                }else {
                    logError("Ocurrio un error con la petición del backup: " + std::string(e.what())
                    + " ruta remota: " + ruta_remota + " ruta sistema: " + archivo.string(), "sentinel.log");
                }
            }
            catch (const ErrorBackupRED& e) {
                logError("Ocurrio un error con la red al intentar realizar el backup a la nube" + std::string(e.what())
                + " ruta remota: " + ruta_remota + " ruta sistema: " + archivo.string(), "sentinel.log");
            }
            catch (const DaemonError& e) {
                logError("Ocurrio un error inesperado: " + std::string(e.what()), "sentinel.log");
            }
        }
    }
    logInfo("Se compelto el backup a DropBox de forma correcta", "sentinel.log");
}