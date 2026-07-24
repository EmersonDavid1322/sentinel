#include <string>
#include <fstream>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <sys/statvfs.h>
#include "errores.h"
#include "monitor.h"
#include "config.h"
#include "logger.h"
#include "notificador.h"
#include "sentinel_estado.h"

double uso_ram() {
    int intentos = 3;
    while (intentos > 0) {
        std::ifstream archivo("/proc/meminfo");
    
        if (!archivo.is_open()) {
            throw ErrorMonitor("No se pudo obtener los datos de la memoria ram en la carpeta: '/proc/meminfo'"); 
        }

        std::string etiqueta;
        long long valor = 0;
        std::string unidad;

        long long memTotal = 0;
        long long memAvailable = 0;

        while (archivo >> etiqueta >> valor >> unidad) {
            if (etiqueta == "MemTotal:") {
                memTotal = valor;
            } 
            else if (etiqueta == "MemAvailable:") {
                memAvailable = valor;
            }
            if (memTotal > 0 && memAvailable > 0) {
                break;
            }
        }
        
        archivo.close();

        if (memTotal > 0 && memAvailable > 0){
            long long memUsada = memTotal - memAvailable;

            double porcentajeUso = (memUsada * 100.0) / memTotal;

            return porcentajeUso;
        }
        intentos --;

        if (intentos == 0) {
            throw ErrorMonitor("El resultado del uso de momeoria ram fue 0"); 
        }
    }
    throw ErrorMonitor("Error inesperado en uso_ram");
}

bool obtener_tics_cpu(long long& trabajo, long long& descanso) {
    std::ifstream archivo("/proc/stat");
    if (!archivo.is_open()) {
        throw ErrorMonitor("No se pudo obtener los tics del CPU en la carpeta: '/proc/stat'");
    }

    std::string etiqueta;
    archivo >> etiqueta;

    long long user, nice, system, idle, iowait, irq, softirq;

    if (archivo >> user >> nice >> system >> idle >> iowait >> irq >> softirq) {

        trabajo = user + nice + system + iowait + irq + softirq;

        descanso = idle;
        return true;
    }
    throw ErrorMonitor("No se pudo obtener los tics del CPU en la carpeta: '/proc/stat'");
}

double uso_cpu() {
    long long trabajoA = 0, descansoA = 0;
    long long trabajoB = 0, descansoB = 0;

    if (!obtener_tics_cpu(trabajoA, descansoA)) return -1.0;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (!obtener_tics_cpu(trabajoB, descansoB)) return -1.0;

    long long deltaTrabajo = trabajoB - trabajoA;
    long long deltaDescanso = descansoB - descansoA;
    long long deltaTotal = deltaTrabajo + deltaDescanso;

    if (deltaTotal == 0) return 0.0;

    double porcentajeUso = (deltaTrabajo * 100.0) / deltaTotal;

    return porcentajeUso;
}

double uso_disco() {
    struct statvfs datos;

    if (statvfs("/", &datos) != 0) {
        throw ErrorMonitor("No se podido analizar el uso del disco error al usar la estrcutura 'statvfs'");
    }
    
    unsigned long long bloquesTotales = datos.f_blocks; 
    unsigned long long bloquesLibres = datos.f_bavail;   

    unsigned long long bloquesUsados = bloquesTotales - bloquesLibres;

    if (bloquesTotales == 0) return 0.0;

    double porcentajeUso = (bloquesUsados * 100.0) / bloquesTotales;

    return porcentajeUso;
}


void revisarLimites(const ConfigMonitor& config ,  const MetricasSistema& metricas){
    std::string mensaje;

    if (metricas.ram >= config.ram){
        int ram_entero = static_cast<int>(metricas.ram);
        logWarning("Monitor: se sobrepaso el limite de uso en la memoria ram: " + std::to_string(ram_entero) + "%");
        mensaje +=  "\nSe sobrepaso el limite de la ram: " + std::to_string(ram_entero) + "%";
    }

    if (metricas.cpu >= config.cpu){
        int cpu_entero = static_cast<int>(metricas.cpu);
        logWarning("Monitor: se sobrepaso el limite de uso del cpu: " + std::to_string(cpu_entero) + "%");
        mensaje += "\nSe sobrepaso el limite del CPU: " + std::to_string(cpu_entero) + "%";
    }

    if (metricas.disco >= config.disco){
        int disco_entero = static_cast<int>(metricas.disco);
        logWarning("Monitor: se sobrepaso el limite de espacio en el disco: " + std::to_string(disco_entero) + "%");
        mensaje += "\nSe sobrepaso el limite de espacio disco: " + std::to_string(disco_entero) + "%";
    }
    if (!mensaje.empty()){
        enviarNotificación("Monitor", mensaje, "WARNING");
    }
}

void ejecutarMonitoreo(const ConfigMonitor& config){
    try{
        MetricasSistema metricas;
        metricas.ram = uso_ram();
        metricas.cpu = uso_cpu();
        metricas.disco = uso_disco();

        revisarLimites(config, metricas);
    }
    catch(const ErrorMonitor& e){
        logError("Error en monitor - " + std::string(e.what()));
        enviarNotificación("Error Monitor", "Ocurrio un error en el intento de telemetrica: " + std::string(e.what()), "ERROR");
    }
    catch(const DaemonError& e){
        logError("Error en monitor - " + std::string(e.what()));
        enviarNotificación("Error Deamon-Monitor", "Ocurrio un error en el intento de telemetrica: " + std::string(e.what()), "ERROR");
    }
}

void loopMonitor(ConfigCompartida& config_compartida) {
    while (corriendo) {
        ConfigSentinel config = config_compartida.obtener();

        if (config.monitor.activo) {
            ejecutarMonitoreo(config.monitor);
        }

        std::unique_lock<std::mutex> lock(mtx_apagado);
        cv_apagado.wait_for(lock, std::chrono::seconds(60), [] { return !corriendo.load(); });
    }
}