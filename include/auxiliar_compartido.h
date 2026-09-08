#pragma once
#include <filesystem>
#include <vector>
namespace fs = std::filesystem;

bool verificarHoraBackup(const std::string& horaConfigurada);

bool debeIgnorarce(const fs::path& ruta, const std::vector<std::string>& lista_ignorar);

void limpiarLog();

bool archivoModificadoCreadoHoy(const fs::path& ruta);

std::string obtenerNombreCarpetaBackup();

void guardarNombreUltimoBackup(const std::string& parametro, const std::string& nombre);