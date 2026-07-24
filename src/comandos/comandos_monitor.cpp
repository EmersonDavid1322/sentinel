#include "comandos_monitor.h"
#include "rutas.h"
#include "comandos_auxiliar.h"
#include "procesar_comandos.h"
#include "monitor.h"
#include "errores.h"
#include <string>
#include <filesystem>

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
        enviarRespuesta("Resultado del consumo actual\nCPU: " + std::to_string(uso_cpu()) + "%"
                        + "\nRAM: " + std::to_string(uso_ram())+ "%" + "\nDisco: " + std::to_string(uso_disco()) + "%");
    }
    catch(const ErrorMonitor& e){
        enviarRespuesta("Error Monitor Ocurrio un error en el intento de telemetrica: " + std::string(e.what()));
    }
    catch(const DaemonError& e){
        enviarRespuesta("Error Deamon-Monitor Ocurrio un error en el intento de telemetrica: " + std::string(e.what()));
    }
}