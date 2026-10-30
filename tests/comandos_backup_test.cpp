#include <gtest/gtest.h>
#include "comandos_backup.h"
#include "backup.h"

//Test respuesta a la funcion hacerBackup
struct RespuestaEspetada {
    std::string nombre;
    std::string mensaje_esperado;
    ResultadoHacerBackup dado;
};

class RespuestaResultadoBackupTest
    : public ::testing::TestWithParam<RespuestaEspetada> {
};

TEST_P(RespuestaResultadoBackupTest, respuestas) {
    const auto& respuesta = GetParam();

    EXPECT_EQ(mensajeResultadoBackup(respuesta.dado),respuesta.mensaje_esperado);
}

INSTANTIATE_TEST_SUITE_P(
    CasosIgnorar,
    RespuestaResultadoBackupTest,
    ::testing::Values(
        RespuestaEspetada{
            "RespuestaExitosa",
            "El backup se a realizado con existo con la configuración establecida",
            {ResultadoBackupEnum::EXITO}
        },
        RespuestaEspetada{
            "RespuestaCanceladaCPU",
            "El backup a sido cancelado debido a que se detecto un alto uso de cpu (Active ´forzar_backup' si desea que no se cancele el backup)",
            {ResultadoBackupEnum::CANCELADO_CPU}
        },
        RespuestaEspetada{
            "RespuestaCanceladaDisco",
            "El backup a sido cancelado debido a que se detecto que se sobrepasa el limite de espacio en el dispositivo (Active ´forzar_backup' si desea que no se cancele el backup)",
            {ResultadoBackupEnum::CANCELADO_DISCO}
        },
        RespuestaEspetada{
            "RespuestaFallidoDestinoNoAccesible",
            "El backup a fallado debido a que no se puedo acceder al destino actual",
            {ResultadoBackupEnum::FALLIDO, CausaBackup::DESTINO_INACCESIBLE}
        },
        RespuestaEspetada{
            "RespuestaFallidoCarpetaInexistente",
            "El backup a fallado debido a que hay una carpeta inexistente en la lista de carpetas a copiar",
            {ResultadoBackupEnum::FALLIDO, CausaBackup::CARPETA_NO_EXISTE}
        },
        RespuestaEspetada{
            "RespuestaFallidoConfiguracionesNoValidas",
            "EL backup a fallado debido a que la configuración actual es invalida",
            {ResultadoBackupEnum::FALLIDO, CausaBackup::CONFIG_INVALIDA}
        },
        RespuestaEspetada{
            "RespuestaCompletadoConErrores",
            "El backup se a finalizado con errores, revise los logs para información detallada",
            {ResultadoBackupEnum::COMPLETADO_CON_ERRORES}
        },
        RespuestaEspetada{
            "RespuestaFallidoSinCausa",
            "El backup a dado resultado fallido y no se a encontrado una causa registrada",
            {ResultadoBackupEnum::FALLIDO}
        },
        RespuestaEspetada{
            "RespuestaFallidoDesconocido",
             "El backup a fallado debido a una causa desconocida",
            {ResultadoBackupEnum::FALLIDO, CausaBackup::DESCONOCIDA}
        }
    ),
    [](const testing::TestParamInfo<RespuestaEspetada>& info) {
        return info.param.nombre;
    }
);