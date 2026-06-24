#pragma once
#include <string>
#include <vector>

void verificarCarpetas(const std::vector<std::string>& carpetas, const std::string& destino);

void ejecutarBackup(const std::vector<std::string>& carpetas, const std::string& destino);

void registroResultado(const std::string& resultado);

void hacerBackup(const std::vector<std::string>& carpetas, const std::string& destino, const std::string hora);