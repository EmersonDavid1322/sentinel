cmake_minimum_required(VERSION 3.10)
project(sentinel)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Wall -Wextra -Wpedantic)

find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(LIBNOTIFY REQUIRED IMPORTED_TARGET libnotify)

# ejecutable principal
add_executable(sentinel
        src/main.cpp
        src/config_json/config_loader.cpp
        src/rutas.cpp
        src/logger.cpp
        src/config_json/sentinel_config.cpp
        src/backups/backup_local/backup.cpp
        src/monitor/monitor.cpp
        src/organizer/organizer.cpp
        src/notificador.cpp
        src/sentinel_estado.cpp
        src/comandos/procesar_comandos.cpp
        src/comandos/comandos_auxiliar.cpp
        src/comandos/comandos_backup.cpp
        src/comandos/comandos_monitor.cpp
        src/comandos/comandos_organizador.cpp
        src/comandos/comandos_estado.cpp
        src/backups/backup_nube/backup_nube_subida.cpp
        src/backups/backup_nube/backup_nube_listado.cpp
        src/comandos/comandos_backup_nube.cpp
        src/backups/backup_nube/backup_nube_auxiliar_dropbox.cpp
        src/backups/backup_nube/backup_nube_bajada.cpp
        src/backups/backup_nube/loop_backup_nube.cpp
        src/backups/backup_nube/backup_nube_auxiliar.cpp
        src/backups/backups_auxiliares/auxiliar_compartido.cpp
        src/backups/backup_local/auxiliar_backup_local.cpp
)

target_include_directories(sentinel PRIVATE include)
target_link_libraries(sentinel
        PRIVATE
        CURL::libcurl
        Threads::Threads
        PkgConfig::LIBNOTIFY
)

# =============================================================================
# CONFIGURACIÓN DE PRUEBAS (TESTS)
# =============================================================================
enable_testing()

find_package(GTest REQUIRED)

#ejecutable de pruebas
add_executable(sentinel_test
        tests/backup_auxiliar_test.cpp
        tests/backup_test.cpp
        src/backups/backups_auxiliares/auxiliar_compartido.cpp
        src/backups/backup_nube/backup_nube_auxiliar.cpp
        src/backups/backup_local/backup.cpp
        src/backups/backup_local/auxiliar_backup_local.cpp
        src/monitor/monitor.cpp
        src/sentinel_estado.cpp
        src/logger.cpp
        src/notificador.cpp
        src/rutas.cpp
        tests/sentinel_config_test.cpp
        src/config_json/sentinel_config.cpp
        src/config_json/config_loader.cpp
        tests/rutas_test.cpp
)

# Vincular librerías al ejecutable de pruebas
target_include_directories(sentinel_test PRIVATE include)
target_link_libraries(sentinel_test
        PRIVATE
        GTest::gtest_main
        Threads::Threads
        CURL::libcurl
        PkgConfig::LIBNOTIFY
)

include(GoogleTest)
gtest_discover_tests(sentinel_test)

target_compile_options(sentinel_test PRIVATE
        --coverage
        -fprofile-arcs
        -ftest-coverage
)

target_link_options(sentinel_test PRIVATE
        -fprofile-arcs
        -ftest-coverage
)
