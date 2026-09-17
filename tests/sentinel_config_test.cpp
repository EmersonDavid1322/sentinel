#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "sentinel_config.h"
#include "errores.h"
#include "json.hpp"
namespace fs = std::filesystem;
using json = nlohmann::json;

class VerificarConfiguracionesTests : public ::testing::Test {
protected:
    fs::path ruta_prueba;

    void SetUp() override{
        ruta_prueba = fs::temp_directory_path() / "sentinel_dir_test";

        fs::remove_all(ruta_prueba);
        fs::create_directories(ruta_prueba);
    }

    void TearDown() override {
        fs::remove_all(ruta_prueba);
    }
};

//Función de Inicializar configuraciones
TEST_F(VerificarConfiguracionesTests, ConfigNoExistentes) {
    fs::path ruta_config_test = ruta_prueba / "sentinel.json";

    EXPECT_THROW(inicializarConfiguraciones(ruta_config_test), ErrorConfig);
}

TEST_F(VerificarConfiguracionesTests, ConfigIncompleto) {
    fs::path ruta_config_test = ruta_prueba / "sentinel.json";
    std::ofstream config(ruta_config_test);

    json plantilla_organizador = R"({
        "organizador": {
            "carpeta_vigilar": "",
            "activo": false,
            "reglas": {}
        }
    })"_json;

    config << plantilla_organizador.dump(4);
    config.close();

    json config_defecto = obtenerConfiguracionPorDefectoCompleta();
    json config_test;

    inicializarConfiguraciones(ruta_config_test);

    std::ifstream config_lectura(ruta_config_test);
    config_lectura >> config_test;

    EXPECT_TRUE(config_defecto == config_test);
}

//Función crear configuración por defecto
TEST_F(VerificarConfiguracionesTests, CrearConfiguraciones) {
    fs::path ruta_config_test = ruta_prueba / "sentinel.json";

    crearConfigPorDefecto(ruta_config_test);

    json config_defecto = obtenerConfiguracionPorDefectoCompleta();
    json config_test;

    std::ifstream config_lectura(ruta_config_test);
    config_lectura >> config_test;
    config_lectura.close();

    EXPECT_TRUE(config_defecto == config_test);
}

TEST_F(VerificarConfiguracionesTests, ErrorCrearConfiguraciones) {
    fs::path ruta_config_test = "/abc/sentinel.json";

    EXPECT_THROW(crearConfigPorDefecto(ruta_config_test), ErrorConfig);
}

//Función verificar que configuraciones existe
TEST_F(VerificarConfiguracionesTests, SinCarpetaNiConfiguracion) {
    fs::path ruta_config_test = "config_test/sentinel.json";

    json config_defecto = obtenerConfiguracionPorDefectoCompleta();
    json config_test;

    asegurarConfigExiste(ruta_config_test);

    std::ifstream config_lectura(ruta_config_test);
    config_lectura >> config_test;
    config_lectura.close();
    EXPECT_TRUE(config_defecto == config_test);
}

TEST_F(VerificarConfiguracionesTests, ErrorRutaNoAccesible) {
    fs::path ruta_config_test = "/abc/config_test/sentinel.json";

    EXPECT_THROW(asegurarConfigExiste(ruta_config_test), ErrorConfig);
}