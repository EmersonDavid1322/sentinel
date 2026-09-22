#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include "backup_nube_subida.h"
#include "backup_nube_auxiliar_dropbox.h"
#include "sentinel_estado.h"
namespace fs = std::filesystem;

class BackupNubeTest : public ::testing::Test {
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

TEST_F(BackupNubeTest, RealizarBackupNube) {
    modo_test = true;
    const char* clienteID = std::getenv("CLIENTE_ID");
    const char* clienteSecret = std::getenv("CLIENTE_SECRET");
    const char* refresh_token = std::getenv("REFRESH_TOKEN");

    int numero;
    fs::path carpeta_test_1 = ruta_prueba /"carpeta_test_1";
    fs::path carpeta_test_2 = ruta_prueba / "carpeta_test_2";
    fs::path subcarpeta_test = carpeta_test_1 / "subcarpeta_test";
    std::vector<fs::path> carpetas_test{carpeta_test_1, carpeta_test_2};
    std::vector<std::string> carpetas_test_strings{carpeta_test_1.string(), carpeta_test_2.string()};

    fs::create_directories(carpeta_test_1);
    fs::create_directories(carpeta_test_2);
    fs::create_directories(subcarpeta_test);

    std::vector<std::string> archivos{
        carpeta_test_1 / "archivo_test_1.txt", carpeta_test_1 / "archivo_test_2.txt", carpeta_test_1 / "archivo_test_3.txt",
        carpeta_test_2 / "archivo_test_1.txt", carpeta_test_2 / "archivo_test_2.txt", carpeta_test_2 / "archivo_test_3.txt",
        subcarpeta_test / "archivo_test_1.txt", subcarpeta_test / "archivo_test_2.txt", subcarpeta_test / "archivo_test_3.txt"
    };

    for (const auto& archivo : archivos) {
        std::ofstream archivo_test(archivo);
        ++numero;
        archivo_test << "Archivo test numero: " + std::to_string(numero);
    }

    ConfigBackupNube configuraciones_test{
        carpetas_test_strings,
        "/backupTEST",
        {},
        "ff1122",
        clienteID,
        clienteSecret,
        refresh_token,
        "00:00",
        "00:00",
        "",
        true,
        false,
        false,
        false,
        false
    };

    ejecutarBackupNube(configuraciones_test);
    std::string token = renovarAccessToken(configuraciones_test);

    for (const auto& carpeta : carpetas_test) {
        for (const auto& entrada : fs::directory_iterator(carpeta)) {
            fs::path ruta_relativa = fs::relative(entrada.path(), carpeta.parent_path());

            fs::path dropboxpath = configuraciones_test.carpeta_remota / ruta_relativa;

            EXPECT_TRUE(verificarSiExisteArchivoDropbox(token, dropboxpath));
        }
    }
    modo_test = false;
}