#include "comandos_organizador.h"
#include "comandos_auxiliar.h"
#include "rutas.h"
#include <string>
#include <filesystem>
#include "errores.h"
#include "logger.h"
namespace fs = std::filesystem;

std::string agregarReglaOrganizador(const std::string& extension, const std::string& carpetaDestino) {

    std::filesystem::path ruta_destino(carpetaDestino);
    if (!fs::exists(ruta_destino)) {
        return "La carpeta destino " + carpetaDestino + " no existe";
    }

    std::filesystem::path ruta = obtenerRutaConfig();
    json datos = leerJSONActual(ruta);

    std::map<std::string, std::string> reglas = datos["organizador"]["reglas"];
    reglas[extension] = carpetaDestino;
    datos["organizador"]["reglas"] = reglas;

    guardarJSON(datos, ruta);
    return "Regla agregada: " + extension + " -> " + carpetaDestino;
}

std::string procesarComandoOrganizadorAgregarRegla(const std::string& valor) {
    try {
        std::istringstream stream(valor);
        std::string extension, direccion;

        std::getline(stream, extension, '|');
        std::getline(stream, direccion, '|');

        extension = limpiarEspacios(extension);
        direccion = limpiarEspacios(direccion);

        if (extension.empty() || direccion.empty()) {
            return "Formato incorrecto. Usa: extension|direccion (ej: .pdf|/home/usuario/PDFs)";
        }

        if (extension.front() != '.') {
            return "La extension debe comenzar con un punto, ej: .pdf";
        }

        return agregarReglaOrganizador(extension, direccion);
    }
    catch (const ErrorConfig& e) {
        logError("Ocurrio un error de configuración durante el proceso del comando 'agregar_regla': " + std::string(e.what()), "sentinel.log");
        return "Ocurrio un error de configuración durante el proceso del comando 'agregar_regla': " + std::string(e.what());
    }
}
