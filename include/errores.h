#pragma once
#include <stdexcept>
#include <string>

enum class CausaBackup {
    CONFIG_INVALIDA,
    CARPETA_NO_EXISTE,
    DESTINO_INACCESIBLE,
    DESCONOCIDA
};

class DaemonError : public std::runtime_error {
public:
    DaemonError(const std::string& mensaje)
        : std::runtime_error(mensaje) {}
};

class ErrorBackup : public DaemonError {
public:
    CausaBackup causa;
    ErrorBackup(const std::string& mensaje,  CausaBackup causa_dada = CausaBackup::DESCONOCIDA)
        : DaemonError(mensaje), causa (causa_dada) {}
};

class ErrorBackupRED : public DaemonError {
public:
    ErrorBackupRED(const std::string& mensaje)
        : DaemonError(mensaje) {}
};

class ErrorBackupAPI : public DaemonError {
public:
    long codigoHTTP;

    ErrorBackupAPI(const std::string& mensaje, long codigo)
        : DaemonError(mensaje), codigoHTTP(codigo) {}
};

class ErrorMonitor : public DaemonError {
public:
    ErrorMonitor(const std::string& mensaje)
        : DaemonError(mensaje) {}
};

class ErrorOrganizador : public DaemonError {
public:
    ErrorOrganizador(const std::string& mensaje)
        : DaemonError(mensaje) {}
};
class ErrorInotify : public DaemonError {
public:
    ErrorInotify(const std::string& mensaje) 
        : DaemonError(mensaje) {}
};
class ErrorConfig : public DaemonError {
public:
    ErrorConfig(const std::string& mensaje) : DaemonError(mensaje) {}
};