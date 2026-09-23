#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include "backup_nube_subida.h"
#include "backup_nube_auxiliar_dropbox.h"
#include "sentinel_estado.h"
#include "auxiliar_compartido.h"
namespace fs = std::filesystem;

struct DatosRutas {
    std::string clienteID;
    std::string clienteSecret;
    std::string refreshToken;
    std::vector<std::string> carpetas_test_strig;
};

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

DatosRutas PrepararBackupNub (const std::filesystem::path& ruta_prueba) {
    const char* clienteID = std::getenv("CLIENTE_ID");
    const char* clienteSecret = std::getenv("CLIENTE_SECRET");
    const char* refresh_token = std::getenv("REFRESH_TOKEN");

    fs::path carpeta_test_1 = ruta_prueba /"carpeta_test_1";
    fs::path carpeta_test_2 = ruta_prueba / "carpeta_test_2";
    fs::path subcarpeta_test = carpeta_test_1 / "subcarpeta_test";
    std::vector<std::string> carpetas_test_strings{carpeta_test_1.string(), carpeta_test_2.string()};

    fs::create_directories(carpeta_test_1);
    fs::create_directories(carpeta_test_2);
    fs::create_directories(subcarpeta_test);

    std::vector<std::string> archivos{
        carpeta_test_1 / "archivo_test_1.txt", carpeta_test_1 / "archivo_test_2.txt", carpeta_test_1 / "archivo_test_3.txt",
        carpeta_test_2 / "archivo_test_1.txt", carpeta_test_2 / "archivo_test_2.txt", carpeta_test_2 / "archivo_test_3.txt",
        subcarpeta_test / "archivo_test_1.txt", subcarpeta_test / "archivo_test_2.txt", subcarpeta_test / "archivo_test_3.txt",
        //archivos secundarios
        subcarpeta_test / "archivo_log_test.log", carpeta_test_1 / "archivo_json_test.json"
    };

    for (const auto& archivo : archivos) {
        std::ofstream archivo_test(archivo);
        archivo_test << "Archivo test";
    }

    return {
        clienteID,
        clienteSecret,
       refresh_token,
        carpetas_test_strings
    };
}

TEST_F(BackupNubeTest, RealizarBackupNube) {
    modo_test = true;
    DatosRutas datos = PrepararBackupNub(ruta_prueba);

    ConfigBackupNube configuraciones_test{
        datos.carpetas_test_strig,
        "/backupTEST/TEST1",
        {},
        "ff1122",
        datos.clienteID,
        datos.clienteSecret,
        datos.refreshToken,
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

    for (const auto& carpeta : datos.carpetas_test_strig) {
        for (const auto& entrada : fs::directory_iterator(carpeta)) {
            fs::path ruta_carpeta = carpeta;
            fs::path ruta_relativa = fs::relative(entrada.path(), ruta_carpeta.parent_path());

            fs::path dropboxpath = configuraciones_test.carpeta_remota / ruta_relativa;

            EXPECT_TRUE(verificarSiExisteArchivoDropbox(token, dropboxpath));
        }
    }
    elimarAnteriorBackupNube(configuraciones_test.carpeta_remota, token);
    modo_test = false;
}

TEST_F(BackupNubeTest, RealizarBackupNubeCreandoCarpeta) {
    modo_test = true;
    DatosRutas datos = PrepararBackupNub(ruta_prueba);

    ConfigBackupNube configuraciones_test{
        datos.carpetas_test_strig,
        "/backupTEST/TEST2",
        {},
        "ff1122",
        datos.clienteID,
        datos.clienteSecret,
        datos.refreshToken,
        "00:00",
        "00:00",
        "",
        true,
        false,
        false,
        true,
        false
    };
    std::string nombre_carpeta = obtenerNombreCarpetaBackup();
    ejecutarBackupNube(configuraciones_test);
    std::string token = renovarAccessToken(configuraciones_test);

    for (const auto& carpeta : datos.carpetas_test_strig) {
        for (const auto& entrada : fs::directory_iterator(carpeta)) {
            fs::path ruta_carpeta = carpeta;
            fs::path ruta_relativa = fs::relative(entrada.path(), ruta_carpeta.parent_path());

            fs::path dropboxpath = configuraciones_test.carpeta_remota +  "/" + nombre_carpeta + "/" + ruta_relativa.string();

            EXPECT_TRUE(verificarSiExisteArchivoDropbox(token, dropboxpath));
        }
    }
    elimarAnteriorBackupNube(configuraciones_test.carpeta_remota, token);
    modo_test = false;
}

TEST_F(BackupNubeTest, RealizarBackupNubeIgnorandoArchivos) {
    modo_test = true;
    DatosRutas datos = PrepararBackupNub(ruta_prueba);

    ConfigBackupNube configuraciones_test{
        datos.carpetas_test_strig,
        "/backupTEST/TEST3",
        {".txt"},
        "ff1122",
        datos.clienteID,
        datos.clienteSecret,
        datos.refreshToken,
        "00:00",
        "00:00",
        "",
        true,
        false,
        false,
        true,
        false
    };

    std::string nombre_carpeta = obtenerNombreCarpetaBackup();
    ejecutarBackupNube(configuraciones_test);
    std::string token = renovarAccessToken(configuraciones_test);

    for (const auto& carpeta : datos.carpetas_test_strig) {
        for (const auto& entrada : fs::directory_iterator(carpeta)) {
            fs::path ruta_carpeta = carpeta;
            fs::path ruta_relativa = fs::relative(entrada.path(), ruta_carpeta.parent_path());

            fs::path dropboxpath = configuraciones_test.carpeta_remota +  "/" + nombre_carpeta + "/" + ruta_relativa.string();

            if (dropboxpath.extension() == ".txt") {
                EXPECT_FALSE(verificarSiExisteArchivoDropbox(token, dropboxpath));
            }else {
                EXPECT_TRUE(verificarSiExisteArchivoDropbox(token, dropboxpath));
            }
        }
    }
    elimarAnteriorBackupNube(configuraciones_test.carpeta_remota, token);
    modo_test = false;
}