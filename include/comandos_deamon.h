#pragma once
#include "json.hpp"
#include "config_loader.h"
using json = nlohmann::json;

//json
json leerJSONActual(const std::filesystem::path& ruta);

void guardarJSON(const json& datos, const std::filesystem::path& ruta);

//estado
void cambiarEstadoSeccion(const std::string& seccion, bool activo);

//auxiliar
std::string limpiarEspacios(const std::string& texto);

void cambiarDireccion(const std::string& parametro,const std::string& llave , const std::string& dirrecion);

//backup
void agregarCarpetaBackup(const std::string& carpeta);

void ejecutarBackupComando(const ConfigBackup& configBackup);

//monitor
void cambiarValorMonitor(const std::string& parametro, double valor);

void ejecutarCambioValorMonitor(const std::string& parametro, std::string& valor);

void ejecutarMonitoreoComando();

//organizador
void agregarReglaOrganizador(const std::string& extension, const std::string& carpetaDestino);

void procesarComandoOrganizadorAgregarRegla(const std::string& valor);

//estado
std::string estadoBackup(const ConfigBackup& config);

std::string  estadoMonitor(const ConfigMonitor& config);

std::string  estadoOrganizador(const ConfigOrganizador& config);

std::string generarDiagnostico(const ConfigSentinel& config);