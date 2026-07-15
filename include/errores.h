#pragma once
#include <stdexcept>
#include <string>

class DaemonError : public std::runtime_error {
public:
    DaemonError(const std::string& mensaje)
        : std::runtime_error(mensaje) {}
};

class ErrorBackup : public DaemonError {
public:
    ErrorBackup(const std::string& mensaje)
        : DaemonError(mensaje) {}
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