#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <sys/statvfs.h>
#include "errores.h"
#include "monitor.h"
#include "config.h"
#include "backup.h"

double uso_ram() {
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

    if (memTotal == 0) {
        throw ErrorMonitor("El resultado del uso de momeoria ram fue 0"); 
    }

    long long memUsada = memTotal - memAvailable;

    double porcentajeUso = (memUsada * 100.0) / memTotal;

    return porcentajeUso;
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
        ErrorMonitor("No se podido analizar el uso del disco error al usar la estrcutura 'statvfs'");
    }
    
    unsigned long long bloquesTotales = datos.f_blocks; 
    unsigned long long bloquesLibres = datos.f_bavail;   

    unsigned long long bloquesUsados = bloquesTotales - bloquesLibres;

    if (bloquesTotales == 0) return 0.0;

    double porcentajeUso = (bloquesUsados * 100.0) / bloquesTotales;

    return porcentajeUso;
}


void revisarLimites(std::unordered_map<std::string, int> limites, double& ram, double& cpu, double& disco){
    int ram_entero = static_cast<int>(ram);
    int cpu_entero = static_cast<int>(cpu);
    int disco_entero = static_cast<int>(disco);

    if (ram >= limites["ram"]){
        registroResultado("Monitor: se sobrepaso el limite de uso en la memoria ram: " + std::to_string(ram_entero) + "%");
    }

    if (cpu >= limites["cpu"]){
        registroResultado("Monitor: se sobrepaso el limite de uso del cpu: " + std::to_string(cpu_entero) + "%");
    }

    if (disco >= limites["disco"]){
        registroResultado("Monitor: se sobrepaso el limite de espacio en el disco: " + std::to_string(disco_entero) + "%");
    }
}

void ejecutarMonitoreo(const int& limite_ram, const int& limite_cpu, const int& limite_disco){
    try{
        std::unordered_map<std::string, int> limites;
        limites["ram"] = limite_ram;
        limites["cpu"] = limite_cpu;
        limites["disco"] = limite_disco;

        double ram = uso_ram();
        double cpu = uso_cpu();
        double disco = uso_disco();

        revisarLimites(limites, ram, cpu, disco);

    }
    catch(const ErrorMonitor& e){
        std::cout << "ErrorMonitor: " << e.what() << std::endl;
        registroResultado("Error en monitor - " + std::string(e.what()));
    }
    catch(const DaemonError& e){
        std::cout << "DaemonError: " << e.what() << std::endl;
        registroResultado("Error en monitor - " + std::string(e.what()));
    }
}