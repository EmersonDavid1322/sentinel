#include <gtest/gtest.h>
#include "rutas.h"

TEST(ValidarRutas, RutasSistema) {
    EXPECT_EQ(obtenerRutaConfig(), "/etc/sentinel/sentinel.json");
    EXPECT_EQ(obtenerRutaLogs(),"/var/log/sentinel");
    EXPECT_EQ(obtenerRutaEstado(), "/var/lib/sentinel/");
}