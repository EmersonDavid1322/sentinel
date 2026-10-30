#pragma once
#include <string>
#include <filesystem>
#include <optional>
#include "errores.h"
#include "config_compartida.h"

//enum class para la verificación de recursos al momento de forzar o no el backup
enum class ResultadoVerificacionRecursos {
    OK,
    CANCELADO_DISCO,
    CANCELADO_CPU,
    FORZADO
};

//Resultado de la copias de los archivos junto con la ruta del backup, confirmando con un bool si hubo errores o no
struct ResultadoCopiaBackup {
    bool completado;
    std::filesystem::path ruta_backup;
};

//enum class para el resultado final de la funcion "hacerBackup"
enum class ResultadoBackupEnum {
    EXITO,
    COMPLETADO_CON_ERRORES,
    CANCELADO_CPU,
    CANCELADO_DISCO,
    FALLIDO
};

//Estruct para agrupar el resultado del backup junto la causa si esta es necesaria
struct ResultadoHacerBackup {
    ResultadoBackupEnum resultado_backup;
    std::optional<CausaBackup> causa;
};

ResultadoVerificacionRecursos verificarRecursosBackup(const ConfigBackup& configBackup, const ConfigMonitor& configMonitor);

void validarConfiguracionBackup(const ConfigBackup& configBackup);

ResultadoCopiaBackup copiarCarpetasBackup(const ConfigBackup& configBackup);

bool ejecutarBackup(const ConfigBackup& configBackup);

void registroResultado(const std::string& resultado);

void loopBackup(ConfigCompartida& config_compartida);

ResultadoHacerBackup hacerBackup(const ConfigBackup& config_backup, const ConfigMonitor& config_monitor);