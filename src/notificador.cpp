#include <iostream>
#include <libnotify/notify.h>
#include <filesystem>
#include <fstream>
#include <mutex>
#include "notificador.h"
#include "logger.h"
#include "json.hpp"
#include "sentinel_estado.h"
std::mutex mutex_notificador;

void enviarNotificación(const std::string& titulo, const std::string& mensaje, const std::string& nivel){
    if (!hayEntornoGrafico) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_notificador);
    NotifyUrgency urgencia;
    std::string icon;

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
        logError("Error notificador: " + std::string(error->message), "sentinel.log");
        g_error_free(error);
    }

    g_object_unref(G_OBJECT(notif));
}
