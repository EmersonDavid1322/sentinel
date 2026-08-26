#!/bin/bash
set -e

NOMBRE_USUARIO="${1:-sentinel}"

echo "Configurando Systemd para Modo Servidor..."

# Limpiar rastros previos
if [ -f "/etc/systemd/system/sentinel.service" ]; then
    systemctl disable --now sentinel.service || true
    rm -f "/etc/systemd/system/sentinel.service"
    systemctl daemon-reload
fi

# Crear usuario de sistema si no existe
if ! id "$NOMBRE_USUARIO" &> /dev/null; then
    useradd --system --no-create-home --shell /usr/sbin/nologin "$NOMBRE_USUARIO"
fi

# Asignar permisos de las carpetas de datos al usuario del daemon
chown -R "$NOMBRE_USUARIO:$NOMBRE_USUARIO" /etc/sentinel /var/log/sentinel /var/lib/sentinel

# Crear el servicio global
cat > /etc/systemd/system/sentinel.service <<EOF
[Unit]
Description=Daemon Sentinel (modo servidor)
After=network.target

[Service]
User=$NOMBRE_USUARIO
Group=$NOMBRE_USUARIO
ExecStart=/usr/local/bin/sentinel
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable --now sentinel.service
systemctl status sentinel.service