#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include "backup.h"
#include "errores.h"
namespace fs = std::filesystem;

//validar configuraciones al hacer backup
TEST(ValidarConfiguraciones, ErrorCarpetasVacia) {
    std::vector<std::string> lista_carpetas{};
    std::vector<std::string> lista_ignorar{".log"};

    ConfigBackup config_test{
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
      validarConfiguracionBackup(config_test),
      ErrorBackup
    );
}

TEST(ValidarConfiguraciones, ErrorDestinoVacio) {
    std::vector<std::string> lista_carpetas{"/home/user/carpeta",};
    std::vector<std::string> lista_ignorar{".log"};

    ConfigBackup config_test{
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
      validarConfiguracionBackup(config_test),
      ErrorBackup
    );
}

TEST(ValidarConfiguraciones, ErrorEliminarSinGuardarRegistro) {
    std::vector<std::string> lista_carpetas{"/home/user/carpeta",};
    std::vector<std::string> lista_ignorar{".log"};

    ConfigBackup config_test{
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
      validarConfiguracionBackup(config_test),
      ErrorBackup
    );
}

class BackupTest : public ::testing::Test {
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

TEST_F(BackupTest, RealizarBackupExitosoSinCrearCarpeta) {
    fs::path carpeta_test1(ruta_prueba.string() + "/" + "carpeta_test1");
    fs::path carpeta_test2(ruta_prueba.string() + "/" + "carpeta_test2");
    fs::path subcarpeta(ruta_prueba.string() + "/" + "carpeta_test2" + "/" + "subcarpeta");

    std::vector<std::string> lista_carpetas{
        carpeta_test1.string(),carpeta_test2.string(),subcarpeta.string()
    };

    std::vector<std::string>lista_archivos{
        carpeta_test1.string() + "/" + "archivo_test1.txt", carpeta_test1.string() + "/" + "archivo_test2.txt",
        carpeta_test1.string() + "/" + "archivo_test3.txt",
        carpeta_test2.string() + "/" + "archivo_test1.txt", carpeta_test2.string() + "/" + "archivo_test2.txt",
        carpeta_test2.string() + "/" + "archivo_test3.txt",
        subcarpeta.string() + "archivo_test1.txt", subcarpeta.string() + "/" + "archivo_test2.txt",
        subcarpeta.string() + "/" + "archivo_test3.txt",
    };

    for (const auto& carpeta : lista_carpetas) {
        fs::create_directories(carpeta);
    }

    for (const auto& archivo : lista_archivos) {
        std::ofstream archivo_crear{archivo};
        archivo_crear << "Archivo test";
        archivo_crear.close();
    }

    fs::create_directories(ruta_prueba.string() + "/" + "destino_test");

    ConfigBackup config_test{
        lista_carpetas,
        ruta_prueba.string() + "/" + "destino_test",
        {},
        "00:00",
        false,
        false,
        false,
        false,
        false
    };

    ResultadoCopiaBackup respuesta = copiarCarpetasBackup(config_test);
    EXPECT_TRUE(respuesta.completado == true);

    for (const auto& archivo : lista_archivos) {
        fs::path archivo_test(archivo);
        fs::path nombre_archivo = archivo_test.filename();
        fs::path nombre_carpeta = archivo_test.parent_path().filename();

        std::cout << respuesta.ruta_backup.string() + "/" + nombre_carpeta.string() + "/" + nombre_archivo.string() << std::endl;
        EXPECT_TRUE(fs::exists(respuesta.ruta_backup.string() + "/" + nombre_carpeta.string() + "/" + nombre_archivo.string()));
    }
};

TEST_F(BackupTest, RealizarBackupExitosoConCrearCarpeta) {
    fs::path carpeta_test1(ruta_prueba.string() + "/" + "carpeta_test1");
    fs::path carpeta_test2(ruta_prueba.string() + "/" + "carpeta_test2");
    fs::path subcarpeta(ruta_prueba.string() + "/" + "carpeta_test2" + "/" + "subcarpeta");

    std::vector<std::string> lista_carpetas{
        carpeta_test1.string(),carpeta_test2.string(),subcarpeta.string()
    };

    std::vector<std::string>lista_archivos{
        carpeta_test1.string() + "/" + "archivo_test1.txt", carpeta_test1.string() + "/" + "archivo_test2.txt",
        carpeta_test1.string() + "/" + "archivo_test3.txt",
        carpeta_test2.string() + "/" + "archivo_test1.txt", carpeta_test2.string() + "/" + "archivo_test2.txt",
        carpeta_test2.string() + "/" + "archivo_test3.txt",
        subcarpeta.string() + "archivo_test1.txt", subcarpeta.string() + "/" + "archivo_test2.txt",
        subcarpeta.string() + "/" + "archivo_test3.txt",
    };

    for (const auto& carpeta : lista_carpetas) {
        fs::create_directories(carpeta);
    }

    for (const auto& archivo : lista_archivos) {
        std::ofstream archivo_crear{archivo};
        archivo_crear << "Archivo test";
        archivo_crear.close();
    }

    fs::create_directories(ruta_prueba.string() + "/" + "destino_test");

    ConfigBackup config_test{
        lista_carpetas,
        ruta_prueba.string() + "/" + "destino_test",
        {},
        "00:00",
        false,
        false,
        false,
        true,
        false
    };

    ResultadoCopiaBackup respuesta = copiarCarpetasBackup(config_test);
    EXPECT_TRUE(respuesta.completado == true);

    for (const auto& archivo : lista_archivos) {
        fs::path archivo_test(archivo);
        fs::path nombre_archivo = archivo_test.filename();
        fs::path nombre_carpeta = archivo_test.parent_path().filename();

        std::cout << respuesta.ruta_backup.string() + "/" + nombre_carpeta.string() + "/" + nombre_archivo.string() << std::endl;
        EXPECT_TRUE(fs::exists(respuesta.ruta_backup.string() + "/" + nombre_carpeta.string() + "/" + nombre_archivo.string()));
    }
};