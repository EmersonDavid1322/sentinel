#pragma once
#include <filesystem>
#include <vector>
namespace fs = std::filesystem;

bool debeIgnorarce(const fs::path& ruta, const std::vector<std::string>& lista_ignorar);

void limpiarLog();