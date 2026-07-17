cmake_minimum_required(VERSION 3.10)
project(sentinel)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(LIBNOTIFY REQUIRED libnotify)

add_executable(sentinel
    src/main.cpp
    src/config_json/config_loader.cpp
    src/rutas.cpp
    src/logger.cpp
    src/config_json/sentinel_config.cpp
    src/backup/backup.cpp
    src/monitor/monitor.cpp
    src/organizer/organizer.cpp
    src/notificador.cpp
    src/sentinel_estado.cpp
    src/comandos/comandos.cpp
    src/comandos/comandos_deamon.cpp
)

target_include_directories(sentinel PRIVATE ${LIBNOTIFY_INCLUDE_DIRS})
target_link_libraries(sentinel Threads::Threads ${LIBNOTIFY_LIBRARIES})