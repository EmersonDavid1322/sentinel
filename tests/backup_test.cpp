#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include "backup.h"
#include "errores.h"

TEST(ValidarConfiguraciones, ErrorCarpetasVacia) {
    std::vector<std::string> lista_carpetas{};
    std::vector<std::string> lista_ignorar{".log"};

    ConfigBackup canfig{
        lista_carpetas,
        "/home/user/destino",
        lista_ignorar,
        "00:00",
        false,
        false,
        false,
        true,
        true
    };

    EXPECT_THROW(
      validarConfiguracionBackup(canfig),
      ErrorBackup
    );
}

TEST(ValidarConfiguraciones, ErrorDestinoVacio) {
    std::vector<std::string> lista_carpetas{"/home/user/carpeta",};
    std::vector<std::string> lista_ignorar{".log"};

    ConfigBackup canfig{
        lista_carpetas,
        "",
        lista_ignorar,
        "00:00",
        false,
        false,
        false,
        true,
        true
    };

    EXPECT_THROW(
      validarConfiguracionBackup(canfig),
      ErrorBackup
    );
}

TEST(ValidarConfiguraciones, ErrorEliminarSinGuardarRegistro) {
    std::vector<std::string> lista_carpetas{"/home/user/carpeta",};
    std::vector<std::string> lista_ignorar{".log"};

    ConfigBackup canfig{
        lista_carpetas,
        "/home/user/destino",
        lista_ignorar,
        "00:00",
        false,
        false,
        false,
        false,
        true
    };

    EXPECT_THROW(
      validarConfiguracionBackup(canfig),
      ErrorBackup
    );
}