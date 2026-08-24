cmake_minimum_required(VERSION 3.10)
project(sentinel)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_options(-Wall -Wextra -Wpedantic)

include_directories(include)
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(LIBNOTIFY REQUIRED libnotify)


add_executable(sentinel
    src/main.cpp
    src/config_json/config_loader.cpp
    src/rutas.cpp
    src/logger.cpp
    src/config_json/sentinel_config.cpp
    src/backup_local/backup.cpp
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
    src/backup_nube/backup_nube_subida.cpp
    src/backup_local/backup_auxiliar.cpp
    src/backup_nube/backup_nube_listado.cpp
    src/comandos/comandos_backup_nube.cpp
    src/backup_nube/backup_nube_auxiliar_dropbox.cpp
    src/backup_nube/backup_nube_bajada.cpp
    src/backup_nube/loop_backup_nube.cpp
    src/backup_nube/backup_nube_auxiliar.cpp
)

enable_testing()

find_package(GTest REQUIRED)

add_executable(sentinel_test
        tests/backup_auxiliar_test.cpp
        src/backup_local/backup_auxiliar.cpp
        src/backup_nube/backup_nube_auxiliar.cpp
)

target_link_libraries(sentinel_test
        PRIVATE
        GTest::gtest_main
)

target_include_directories(sentinel_test
        PRIVATE
        include
)

include(GoogleTest)

gtest_discover_tests(sentinel_test)

target_include_directories(sentinel PRIVATE ${LIBNOTIFY_INCLUDE_DIRS})
target_link_libraries(sentinel Threads::Threads ${LIBNOTIFY_LIBRARIES} CURL::libcurl)
target_compile_options(sentinel_test PRIVATE
        --coverage
        -fprofile-arcs
        -ftest-coverage
)

target_link_options(sentinel_test PRIVATE
        -fprofile-arcs
        -ftest-coverage
)