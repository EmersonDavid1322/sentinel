#!/bin/bash
set -e

# Capturar el usuario que viene desde install.sh
NOMBRE_USUARIO="$1"

if [ -z "$NOMBRE_USUARIO" ] || [ "$NOMBRE_USUARIO" == "sentinel" ]; then
    # Por si acaso falló el argumento, intentamos rescatar el usuario real que no sea root
    if [ "$USER" != "root" ]; then
        NOMBRE_USUARIO="$USER"
    else
        echo "Error: Se requiere especificar un usuario gráfico real válido."
        exit 1
    fi
fi

TARGET_UID=$(id -u "$NOMBRE_USUARIO")
TARGET_HOME=$(getent passwd "$NOMBRE_USUARIO" | cut -d: -f6)

echo "Configurando Systemd --user para el usuario gráfico: $NOMBRE_USUARIO (UID: $TARGET_UID)"

# 1. Habilitar Linger de forma nativa
loginctl enable-linger "$NOMBRE_USUARIO"

# 2. Permisos correctos de las carpetas globales
chown -R "$NOMBRE_USUARIO":"$NOMBRE_USUARIO" /etc/sentinel /var/log/sentinel /var/lib/sentinel
chown root:root /usr/local/bin/sentinel /usr/local/bin/sentinel-cli

# 3. Crear rutas de servicio en el HOME real del usuario
DIR_SERVICIOS_USER="$TARGET_HOME/.config/systemd/user"
mkdir -p "$DIR_SERVICIOS_USER"

# 4. Crear el archivo del servicio con el fix de retraso incluido
cat << 'EOF' > "$DIR_SERVICIOS_USER/sentinel.service"
[Unit]
Description=Daemon Sentinel (Modo Escritorio de Usuario)
After=graphical-session.target
BindsTo=graphical-session.target

[Service]
ExecStart=/usr/local/bin/sentinel
KillMode=control-group
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal
Environment=DISPLAY=:0
Environment=XDG_RUNTIME_DIR=/run/user/%U
Environment=DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/%U/bus
ExecStartPre=/usr/bin/sleep 2

[Install]
WantedBy=graphical-session.target
EOF

# Asegurar propiedad de los archivos en el HOME del usuario
chown -R "$NOMBRE_USUARIO":"$NOMBRE_USUARIO" "$TARGET_HOME/.config"

# 5. CONTROL REMOTO NATIVO DE SYSTEMD (El estándar moderno de Arch Linux)
# Usamos el flag --machine para inyectar los comandos directo en el bus del usuario
echo "Recargando demonio de usuario..."
systemctl --user --machine="${NOMBRE_USUARIO}@.host" daemon-reload

echo "Habilitando e iniciando servicio gráfico..."
systemctl --user --machine="${NOMBRE_USUARIO}@.host" enable --now sentinel.service

echo "Verificando estado del servicio..."
systemctl --user --machine="${NOMBRE_USUARIO}@.host" status sentinel.service --no-pager | head -n 15
