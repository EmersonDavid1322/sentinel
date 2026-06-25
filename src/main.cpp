#include <iostream>
#include <vector>
#include <filesystem>
#include <map>
#include "config_loader.h"
#include "errores.h"
#include "json.hpp"
#include "rutas.h"
#include "backup.h"
#include "monitor.h"
namespace fs = std::filesystem;
fs::path ruta_base = obtenerRutaBase();

int main(){
try {
    ConfigSentinel config = cargarConfig(ruta_base / "config" / "sentinel.json");
    
    if (config.monitor.activo) {
        ejecutarMonitoreo(config.monitor.ram, config.monitor.cpu, config.monitor.disco);
    }
} catch (const DaemonError& e) {
    std::cout << "Error critico: " << e.what() << std::endl;
    return 1;
}
return 0;
}