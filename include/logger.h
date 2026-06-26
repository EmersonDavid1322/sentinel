#pragma once

static void escribirLog(const std::string& nivel, const std::string& mensaje);

void logInfo(const std::string& mensaje);
void logWarning(const std::string& mensaje);
void logError(const std::string& mensaje);