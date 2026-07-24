#pragma once
#include <mutex>
#include "config.h"

class ConfigCompartida {
    private:
        ConfigSentinel datos;
        mutable std::mutex mtx;
    public:
    ConfigSentinel obtener() {
        std::lock_guard<std::mutex> lock(mtx);
        return datos;
    }

    void actualizar(const ConfigSentinel& nuevaConfig) {
        std::lock_guard<std::mutex> lock(mtx);
        datos = nuevaConfig;
    }
};