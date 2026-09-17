#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include "backup.h"
#include "errores.h"
namespace fs = std::filesystem;

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

//test Verficarcarpetasbackup
class VerficarBackupTests : public ::testing::Test {
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

TEST_F(VerficarBackupTests, ErrorCarpetaNoExistente) {
    fs::path ruta_test = ruta_prueba / "directorio_no_existente";
    fs::path destino = ruta_prueba / "destino_test";

    fs::create_directories(destino);
    std::vector<std::string> carpetas{ruta_test.string()};

    EXPECT_THROW(verificarCarpetasBackup(carpetas, destino), ErrorBackup);
}

TEST_F(VerficarBackupTests, ErrorDestinoImposibleDeCrear) {
    fs::path ruta_test = ruta_prueba / "directorio_test";
    fs::create_directories(ruta_test);

    fs::path destino = "/abc/destino_test";
    std::vector<std::string> carpetas{ruta_test.string()};

    EXPECT_THROW(verificarCarpetasBackup(carpetas, destino), ErrorBackup);
}

TEST_F(VerficarBackupTests, VerificacionAceptada) {
    fs::path ruta_test = ruta_prueba / "directorio_test";
    fs::path destino_test = ruta_prueba / "destino_test";

    fs::create_directories(ruta_test);

    std::vector<std::string> carpetas{ruta_test.string()};

    EXPECT_EQ(verificarCarpetasBackup(carpetas, destino_test), " " + ruta_test.string());
}