#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "auxiliar_compartido.h"
#include "backup_nube_auxiliar.h"
#include "errores.h"
namespace fs = std::filesystem;

//test para rutas a ignorar utilizado en backup local y nube0
struct CasoIgnorar {
    std::string nombre;
    fs::path ruta;
    std::vector<std::string> reglas;
    bool esperado;
};

class DebeIgnorarseTest
    : public ::testing::TestWithParam<CasoIgnorar> {
};

TEST_P(DebeIgnorarseTest, Casos) {
    const auto& caso = GetParam();

    EXPECT_EQ(
        debeIgnorarce(caso.ruta, caso.reglas),
        caso.esperado
    );
}

INSTANTIATE_TEST_SUITE_P(
    CasosIgnorar,
    DebeIgnorarseTest,
    ::testing::Values(
        CasoIgnorar{
            "IgnorarCarpetaFinal",
            "/home/Emerson/proyecto/.git",
            {".git"},
            true
        },

        CasoIgnorar{
            "IgnorarExtension",
            "/home/Emerson/proyecto/sentinel.log",
            {".log"},
            true
        },

        CasoIgnorar{
            "NoIgnorar",
            "/home/Emerson/proyecto/main.cpp",
            {".log", ".git"},
            false
        },

        CasoIgnorar{
            "IgnorarListaVacia",
            "/home/Emerson/proyecto/main.cpp",
            {},
            false
        },

        CasoIgnorar{
            "IgnorarRutaIntermedia",
            "/home/Emerson/.git/logs/git.log",
            {".git"},
            true
        }
    ),
    [](const testing::TestParamInfo<CasoIgnorar>& info) {
        return info.param.nombre;
    }
);

//Test de la función Calcular ruta local para la bajada de archivos dropbox
struct CasoCacularRutaLocal {
    std::string nombre;
    std::string ruta_archivo;
    std::string ruta_remota;
    std::string ruta_destino;
    std::filesystem::path ruta_esperada;
};

class CalcularRutaLocalTest : public ::testing::TestWithParam<CasoCacularRutaLocal>  {};

TEST_P(CalcularRutaLocalTest, casos) {
    const auto& caso = GetParam();
    std::string ruta_remota = caso.ruta_remota;

    EXPECT_EQ(
        calcularRutaLocal(caso.ruta_archivo, ruta_remota, caso.ruta_destino),
        caso.ruta_esperada
    );
}

INSTANTIATE_TEST_SUITE_P(
    CasoCacularRutaLocal,
    CalcularRutaLocalTest,
    ::testing::Values(
        CasoCacularRutaLocal{
            "BarraInicial",
            "/backupPROXMOX/contendores/100.zip",
            "/backupPROXMOX",
            "/home/backup",
            "/home/backup/contendores/100.zip"
        },
        CasoCacularRutaLocal{
            "SinBarraInicialRutaRemota",
            "/backupPROXMOX/contendores/100.zip",
            "backupPROXMOX",
            "/home/backup",
            "/home/backup/contendores/100.zip"
        }
    ),
    [](const testing::TestParamInfo<CasoCacularRutaLocal>& info) {
        return info.param.nombre;
    }
);

TEST(CalcularRutaLocal, RutaMasCortaQueCarpetaRemotaLanzaExcepcion) {
    std::string ruta_remota = "/backupPROXMOX";

    EXPECT_THROW(
      calcularRutaLocal("/abc", ruta_remota, "/home/destino"),
      ErrorBackup
    );
}

//test filtro archivo backups local/nube
class FiltroArchivosTest : public ::testing::Test {
protected:
    fs::path rutaPrueba;

    void SetUp() override {
        rutaPrueba = fs::temp_directory_path() / "sentinel_tests_dir";

        fs::remove_all(rutaPrueba);
        fs::create_directories(rutaPrueba);
    }

    void TearDown() override {
        fs::remove_all(rutaPrueba);
    }
};

TEST_F(FiltroArchivosTest, IgnorarCarpeta) {
    fs::directory_entry carpeta{rutaPrueba / "carpeta_prueba"};
    fs::create_directories(carpeta.path());

    std::vector<std::string> ignorar {"carpeta_prueba"};

    EXPECT_EQ(
    debeSubirseArchivo(carpeta, ignorar, false),
    FiltroArchivos::IGNORAR_CARPETA
    );
}

TEST_F(FiltroArchivosTest, IgnorarArchivo) {
    fs::directory_entry archivo_ruta{rutaPrueba / "prueba.txt"};
    std::ofstream archivo(archivo_ruta.path().string());

    if (archivo.is_open()) {
        archivo << "Prueba ignorar archivo test filtro";
        archivo.close();
    }

    std::vector<std::string> ignorar {".txt"};

    EXPECT_EQ(
    debeSubirseArchivo(archivo_ruta, ignorar, false),
    FiltroArchivos::IGNORAR
    );
}

TEST_F(FiltroArchivosTest, IgnorarArchivoNoRegular) {
    fs::directory_entry archivo{rutaPrueba / "tuberia"};

    mkfifo(rutaPrueba.c_str(), 0666);

    std::vector<std::string> ignorar{};

    EXPECT_EQ(
    debeSubirseArchivo(archivo, ignorar, false),
    FiltroArchivos::IGNORAR
    );
}

TEST_F(FiltroArchivosTest, AceptarArchivo) {
    fs::directory_entry archivo_ruta{rutaPrueba / "prueba.txt"};
    std::ofstream archivo(archivo_ruta.path().string());

    if (archivo.is_open()) {
        archivo << "Prueba aceptar archivo test filtro";
    }

    std::vector<std::string> ignorar{".log", ".o", ".git", "pruebas"};

    EXPECT_EQ(
    debeSubirseArchivo(archivo_ruta, ignorar, false),
    FiltroArchivos::ACEPTADO
    );
}

TEST_F(FiltroArchivosTest, AceptarArchivoModifcadoHoy) {
    fs::directory_entry archivo_ruta{rutaPrueba / "prueba.txt"};
    std::ofstream archivo(archivo_ruta.path().string());

    if (archivo.is_open()) {
        archivo << "Prueba aceptar archivo test filtro";
    }

    std::vector<std::string> ignorar{".log", ".o", ".git", "pruebas"};

    EXPECT_EQ(
    debeSubirseArchivo(archivo_ruta, ignorar, true),
    FiltroArchivos::ACEPTADO
    );
}