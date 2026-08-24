#include <gtest/gtest.h>
#include "backup_auxiliar.h"
#include "backup_nube_auxiliar.h"
#include "errores.h"

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