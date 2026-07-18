#pragma once
#include <string>

void enviarRespuesta(const std::string& mensaje);

void procesarEstado(std::string modulo, std::string& accion);

void procesarComandoMonitor(std::string& accion, std::string& valor);

void procesarComandoBackup(std::string& accion, std::string& valor);

void procesarComandoOrganizador(std::string& accion, std::string& valor);

void procesarComando(const std::string& comando);

void loopComandos();