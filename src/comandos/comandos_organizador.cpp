#include "comandos_organizador.h"
#include "comandos_auxiliar.h"
#include "procesar_comandos.h"
#include "rutas.h"
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

void agregarReglaOrganizador(const std::string& extension, const std::string& carpetaDestino) {

    std::filesystem::path ruta_destino(carpetaDestino);
    if (!fs::exists(ruta_destino)) {
        enviarRespuesta("La carpeta destino " + carpetaDestino + " no existe");
        return;
    }

    std::filesystem::path ruta = obtenerRutaBase() / "config" / "sentinel.json";
    json datos = leerJSONActual(ruta);

    std::map<std::string, std::string> reglas = datos["organizador"]["reglas"];
    reglas[extension] = carpetaDestino;
    datos["organizador"]["reglas"] = reglas;

    guardarJSON(datos, ruta);
    enviarRespuesta("Regla agregada: " + extension + " -> " + carpetaDestino);
}

void procesarComandoOrganizadorAgregarRegla(const std::string& valor) {
    std::istringstream stream(valor);
    std::string extension, direccion;

    std::getline(stream, extension, '|');
    std::getline(stream, direccion, '|');

    extension = limpiarEspacios(extension);
    direccion = limpiarEspacios(direccion);

    if (extension.empty() || direccion.empty()) {
        enviarRespuesta("Formato incorrecto. Usa: extension|direccion (ej: .pdf|/home/usuario/PDFs)");
        return;
    }

    if (extension.front() != '.') {
        enviarRespuesta("La extension debe comenzar con un punto, ej: .pdf");
        return;
    }

    agregarReglaOrganizador(extension, direccion);
}