#include "comandos_auxiliar.h"
#include <filesystem>
#include <fstream>
#include <string>
#include "errores.h"
#include "json.hpp"
#include "rutas.h"
using json = nlohmann::json;
namespace fs = std::filesystem;
//json carga y guarda
json leerJSONActual(const std::filesystem::path& ruta){
    std::ifstream archivo(ruta);
    if (!archivo.is_open()){
        throw ErrorConfig("No se pudo abir el archivo configuraciones en la ruta: " + ruta.string());
    }
    return json::parse(archivo);
}

void guardarJSON(const json& datos, const std::filesystem::path& ruta){
    std::ofstream archivo(ruta);
    if (!archivo.is_open()){
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

std::string cambiarDireccion(const std::string& parametro,const std::string& llave ,const std::string& dirrecion) {
    std::string destino_limpio = limpiarEspacios(dirrecion);
    if (destino_limpio.empty()) {
        return "No se permiten valores vacios en destino";
    }


    std::filesystem::path direccion_path(destino_limpio);
    if (!fs::exists(direccion_path)) {
        return "La dirrección " + destino_limpio + " no a sido encontrada o no existe";
    }

    std::filesystem::path ruta = obtenerRutaConfig();

    json datos = leerJSONActual(ruta);

    if (datos[parametro][llave] == destino_limpio) {
        return "Esta dirección " + destino_limpio + " ya esta asignada a " + parametro + "-" + llave;
    }

    datos [parametro][llave] = destino_limpio;
    guardarJSON(datos, ruta);
    return "Se a cambiado la dirrecion de: "+ parametro + " a " + destino_limpio;
}

//cambiar estado acitvo modulo
void cambiarEstadoSeccion(const std::string& seccion, bool activo){
    std::filesystem::path ruta = obtenerRutaConfig();

    json datos = leerJSONActual(ruta);
    datos [seccion]["activo"] = activo;
    guardarJSON(datos, ruta);
}

//ignorar para backup local y nube
std::string aniadirIgnorar(const std::string& parametro, const std::string& valor) {
    std::string valor_limpio = limpiarEspacios(valor);
    if (valor_limpio.empty()) {
        return "No se permiten valores vacios en destino";
    }

    std::filesystem::path ruta = obtenerRutaConfig();
    json datos = leerJSONActual(ruta);

    std::vector<std::string> lista_ignorados = datos[parametro]["ignorar"];
    lista_ignorados.push_back(valor_limpio);

    datos[parametro]["ignorar"] = lista_ignorados;
    guardarJSON(datos, ruta);
    return "Se añadido " + valor_limpio + " a la lista de ignorar";
}