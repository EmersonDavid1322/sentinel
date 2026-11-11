#include "comandos_monitor.h"
#include "rutas.h"
#include "comandos_auxiliar.h"
#include "procesar_comandos.h"
#include "monitor.h"
#include "errores.h"
#include <string>
#include <filesystem>
#include "logger.h"

//comandos monitor
void cambiarValorMonitor(const std::string& parametro, double valor){
    std::filesystem::path ruta = obtenerRutaConfig();

    json datos = leerJSONActual(ruta);
    datos ["monitor"][parametro] = valor;
    guardarJSON(datos, ruta);
}

std::string ejecutarCambioValorMonitor(const std::string& parametro, std::string& valor) {
    try{
        double valor_double = std::stod(valor);
        if (valor_double < 0 || valor_double > 100){
            return "El límite debe estar entre 0 y 100.";
        }
        cambiarValorMonitor(parametro, valor_double);
        return "Se cambio el valor del parametro " + parametro + ":" + valor;
    }catch (const std::invalid_argument&){
        return "El valor '" + valor + "' no es un numero valido";
    }catch (const std::out_of_range&){
        return "El valor '" + valor + "' es demasiado grande.";
    }
}

std::string ejecutarMonitoreoComando() {
    try{
        int cpu = static_cast<int>(uso_cpu());
        int ram = static_cast<int>(uso_ram());
        int disco = static_cast<int>(uso_disco());

        return "Resultado del consumo actual\nCPU: " + std::to_string(cpu) + "%"
                        + "\nRAM: " + std::to_string(ram)+ "%" + "\nDisco: " + std::to_string(disco) + "%";
    }
    catch(const ErrorMonitor& e){
        logError("Error Monitor Ocurrio un error en el intento de telemetrica: " + std::string(e.what()), "sentinel.log");
        return "Error Monitor Ocurrio un error en el intento de telemetrica: " + std::string(e.what());
    }
    catch (const ErrorConfig& e) {
        logError("Error Configuraciones Ocurrio un error en el intento de telemetrica: " + std::string(e.what()), "sentinel.log");
        return "Error Configuraciones Ocurrio un error en el intento de telemetrica: " + std::string(e.what());
    }
    catch(const DaemonError& e){
        logError("Error Deamon-Monitor Ocurrio un error en el intento de telemetrica: " + std::string(e.what()), "sentinel.log");
        return "Error Deamon-Monitor Ocurrio un error en el intento de telemetrica: " + std::string(e.what());
    }
}
