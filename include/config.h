#pragma once
#include <string>
#include <vector>
#include <map>
#include "json.hpp"

struct ConfigBackup {
        std::vector<std::string> carpetas;
        std::string destino;
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
        ConfigMonitor monitor;
        ConfigOrganizador organizador;
    };