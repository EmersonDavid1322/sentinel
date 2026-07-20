#include <fstream>
#include "json.hpp"
#include "comandos_deamon.h"
#include "errores.h"
#include "rutas.h"
#include "comandos.h"
#include "monitor.h"
using json = nlohmann::json;

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


//cambiar estado acitvo modulo
void cambiarEstadoSeccion(const std::string& seccion, bool activo){
    std::filesystem::path ruta = obtenerRutaBase() / "config" / "sentinel.json";

    json datos = leerJSONActual(ruta);
    datos [seccion]["activo"] = activo;
    guardarJSON(datos, ruta);
}

//comandos monitor
void cambiarValorMonitor(const std::string& parametro, double valor){
    std::filesystem::path ruta = obtenerRutaBase() / "config" / "sentinel.json";

    json datos = leerJSONActual(ruta);
    datos ["monitor"][parametro] = valor;
    guardarJSON(datos, ruta);
}

void ejecutarCambioValorMonitor(const std::string& parametro, std::string& valor) {
    try{
        double valor_double = std::stod(valor);
        if (valor_double < 0 || valor_double > 100){
            enviarRespuesta("El límite debe estar entre 0 y 100.");
            return;
        }
        cambiarValorMonitor(parametro, valor_double);
        enviarRespuesta("Se cambio el valor del parametro " + parametro + ":" + valor);
    }catch (const std::invalid_argument&){
        enviarRespuesta("El valor '" + valor + "' no es un numero valido");
    }catch (const std::out_of_range&){
        enviarRespuesta("El valor '" + valor + "' es demasiado grande.");
    }
}

void ejecutarMonitoreoComando() {
    try{
        enviarRespuesta("Resultado del consumo actual\nCPU: " + std::to_string(uso_cpu())
                        + "\nRAM: " + std::to_string(uso_ram()) + "\nDisco: " + std::to_string(uso_disco()));
    }
    catch(const ErrorMonitor& e){
        enviarRespuesta("Error Monitor Ocurrio un error en el intento de telemetrica: " + std::string(e.what()));
    }
    catch(const DaemonError& e){
        enviarRespuesta("Error Deamon-Monitor Ocurrio un error en el intento de telemetrica: " + std::string(e.what()));
    }
}