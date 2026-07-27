#include <iostream>
#include <libnotify/notify.h>
#include <filesystem>
#include <fstream>
#include <mutex>
#include "notificador.h"
#include "logger.h"
#include "rutas.h"
#include "json.hpp"
std::mutex mutex_notificador;
namespace fs = std::filesystem;
using json = nlohmann::json;

int verificarEnvio(std::string nivel) {
    std::filesystem::path rutaConfig = obtenerRutaBase() / "config" / "sentinel.json";
    std::ifstream archivo_json(rutaConfig);

    if (!archivo_json.is_open()){
        logError("No se pudo abir el archivo configuraciones en la ruta: " + rutaConfig.string());
    }
    json datos = json::parse(archivo_json);
    std::string opcion_noti = datos["usuaior"]["nivel_noti"];

    if (opcion_noti != "ERRORES" && opcion_noti != "WARNING" && opcion_noti != "NORMAL") {
        logWarning("La configuración de usuario '"  + opcion_noti + "' no coincide con ninguna de las opciones: 'NORMAL', 'BAJO', 'ERRORRES'");
        return 0;
    }

    if (opcion_noti == "NORMAL") {
        return 0;
    }
    if (opcion_noti == "BAJO" && (nivel == "ERROR" || nivel == "WARNING")) {
        return 0;
    }
    if (opcion_noti == "ERRORES" && nivel == "ERROR") {
        return 0;
    }
    else {
        return 1;
    }
}

void enviarNotificación(std::string titulo, std::string mensaje, std::string nivel){
    std::lock_guard<std::mutex> lock(mutex_notificador);
    NotifyUrgency urgencia;
    std::string icon;

    int envio = verificarEnvio(nivel);
    if (envio != 0) {
        logInfo("Se omitio la notificación por configuraciones de usuario de nivel: " + nivel);
        return;
    }

    if (nivel == "ERROR") {
        urgencia = NOTIFY_URGENCY_CRITICAL;
        icon = "dialog-error";
    }
    else if (nivel == "WARNING") {
        urgencia = NOTIFY_URGENCY_NORMAL;
        icon = "dialog-warning";
    }
    else {
        urgencia = NOTIFY_URGENCY_LOW;
        icon = "face-smile";
    }
    
    NotifyNotification* notif = notify_notification_new(
        titulo.c_str(),
        mensaje.c_str(),
        icon.c_str()
    );
    
    notify_notification_set_urgency(notif, urgencia);
    notify_notification_set_timeout(notif, 10000);

    GError* error = nullptr;
    if (!notify_notification_show(notif, &error)){
        logError("Error notificador: " + std::string(error->message));
        g_error_free(error);
    }

    g_object_unref(G_OBJECT(notif));
}