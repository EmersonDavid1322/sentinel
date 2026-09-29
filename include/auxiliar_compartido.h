#pragma once
#include <filesystem>
#include <vector>

enum class FiltroArchivos{
    IGNORAR,
    IGNORAR_CARPETA,
    ACEPTADO
};

bool verificarHoraBackup(const std::string& horaConfigurada);

std::string verificarCarpetasBackup(const std::vector<std::string>& carpetas, const std::string& destino);

void comprobarCarpetasBackup(const std::vector<std::string>& carpetas);

bool debeIgnorarce(const std::filesystem::path& ruta, const std::vector<std::string>& lista_ignorar);

void limpiarLog();

bool archivoModificadoCreadoHoy(const std::filesystem::path& ruta);

std::string obtenerNombreCarpetaBackup();

void guardarRutaUltimoBackup(const std::string& parametro, const std::string& nombre);

std::filesystem::path extraerRutaUltimoBackup(const std::string& parametro);

FiltroArchivos debeSubirseArchivo(const std::filesystem::directory_entry& entrada, const std::vector<std::string>& ignorar, bool soloModificadosHoy);