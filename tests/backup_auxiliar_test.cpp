#include <gtest/gtest.h>
#include "backup_auxiliar.h"

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
    )
);