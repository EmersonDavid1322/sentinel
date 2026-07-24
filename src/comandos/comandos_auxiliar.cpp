#include "comandos_auxiliar.h"
#include <filesystem>
#include <fstream>
#include <string>
#include "procesar_comandos.h"
#include "errores.h"
#include "json.hpp"
#include "monitor.h"
#include "rutas.h"
#include "notificador.h"
using json = nlohmann::json;
namespace fs = std::filesystem;

//json carga y guarda
json leerJSONActual(const std::filesystem::path& ruta){
    std::ifstream archivo(ruta);
    if (!archivo.is_open()){
        enviarRespuesta("No se pudo abir el archivo configuraciones en la ruta: " + ruta.string());
        throw ErrorConfig("No se pudo abir el archivo configuraciones en la ruta: " + ruta.string());
    }
    return json::parse(archivo);
}

void guardarJSON(const json& datos, const std::filesystem::path& ruta){
    std::ofstream archivo(ruta);
    if (!archivo.is_open()){
        enviarRespuesta("No se pudo abir el archivo configuraciones en la ruta: " + ruta.string());
        throw ErrorConfig("No se pudo abir el archivo configuraciones en la ruta: " + ruta.string());
    }
    archivo << datos.dump(4);
}

//auxiliar
std::string limpiarEspacios(const std::string& texto) {
    size_t inicio = texto.find_first_not_of(" \t\r\n");
    if (inicio == std::string::npos) {
        return "";
    }
    size_t final = texto.find_last_not_of(" \t\r\n");
    return texto.substr(inicio, final - inicio + 1);
}

void cambiarDireccion(const std::string& parametro,const std::string& llave ,const std::string& dirrecion) {
    std::string destino_limpio = limpiarEspacios(dirrecion);
    if (destino_limpio.empty()) {
        enviarRespuesta("No se permiten valores vacios en destino");
        return;
    }


    std::filesystem::path direccion_path(destino_limpio);
    if (!fs::exists(direccion_path)) {
        enviarRespuesta("La dirrección " + destino_limpio + " no a sido encontrada o no existe");
        return;
    }

    std::filesystem::path ruta = obtenerRutaBase() / "config" / "sentinel.json";

    json datos = leerJSONActual(ruta);

    if (datos[parametro][llave] == destino_limpio) {
        enviarRespuesta("Esta dirección " + destino_limpio + " ya esta asignada a " + parametro + "-" + llave);
        return;
    }

    datos [parametro][llave] = destino_limpio;
    guardarJSON(datos, ruta);
    enviarRespuesta("Se a cambiado el destino del backup a: " + destino_limpio);
}

//cambiar estado acitvo modulo
void cambiarEstadoSeccion(const std::string& seccion, bool activo){
    std::filesystem::path ruta = obtenerRutaBase() / "config" / "sentinel.json";

    json datos = leerJSONActual(ruta);
    datos [seccion]["activo"] = activo;
    guardarJSON(datos, ruta);
}