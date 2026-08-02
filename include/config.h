#pragma once
#include <string>
#include <vector>
#include <map>
#include "json.hpp"

struct ConfigBackup {
    std::vector<std::string> carpetas;
    std::string destino;
    std::vector<std::string> ignorar;
    std::string hora;
    bool activo;
    bool forzar_backup;
};

struct ConfigBackupNube {
    std::vector<std::string> carpetas;
    std::string carpeta_remota;
    std::vector<std::string> ignorar;
    std::string token;
    std::string hora;
    bool activo;
};

struct ConfigMonitor {
    double cpu;
    double ram;
    double disco;
    bool activo;
};

struct ConfigOrganizador {
    std::string carpeta_vigilar;
    bool activo;
    std::map<std::string, std::string> reglas;
};

struct ConfigSentinel {
    ConfigBackup backup;
    ConfigBackupNube backup_nube;
    ConfigMonitor monitor;
    ConfigOrganizador organizador;
};
struct MetricasSistema {
    double ram;
    double cpu;
    double disco;
};