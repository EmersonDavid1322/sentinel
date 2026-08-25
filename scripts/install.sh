#!/bin/bash
set -e

if [ "$EUID" -eq 0 ]; then
    SUDO=""
else
    SUDO="sudo"
fi

BASE_DIR=$(dirname "$(readlink -f "$0")")
PROYECTO_DIR=$(dirname "$BASE_DIR")

NOMBRE_USUARIO="${1:-sentinel}"

bash "$PROYECTO_DIR/scripts/install_dep.sh"

# limpieza
echo "Deteniendo servicios e instancias previas de Sentinel..."

if systemctl --user is-active --quiet sentinel.service 2>/dev/null; then
    systemctl --user stop sentinel.service || true
fi

# Por si el proceso quedó 'huérfano' suelto en la memoria
if pgrep -x "sentinel" > /dev/null; then
    echo "Detectado proceso huérfano en ejecución. Enviando SIGTERM..."
    pkill -15 -x "sentinel" || true

    sleep 3 

    if pgrep -x "sentinel" > /dev/null; then
        echo "El proceso no respondió al cierre limpio. Forzando SIGKILL..."
        pkill -9 -x "sentinel" || true
    fi
fi

# COMPILACIÓN DEL PROYECTO
echo "Compilando Sentinel..."
mkdir -p "$PROYECTO_DIR/build"
cd "$PROYECTO_DIR/build"
cmake "$PROYECTO_DIR" -DCMAKE_BUILD_TYPE=Release
make

if [ -f "/usr/local/bin/sentinel" ]; then
    echo "Eliminando versión de ejecutable anterior..."
    $SUDO rm "/usr/local/bin/sentinel"
fi

$SUDO cp "$PROYECTO_DIR/build/sentinel" "/usr/local/bin/"

$SUDO chmod +x /usr/local/bin/sentinel

# Limpieza de la carpeta temporal de compilación
rm -rf "$PROYECTO_DIR/build"

# crear carpetas y archivos
$SUDO mkdir -p "/etc/sentinel/"
$SUDO mkdir -p "/var/log/sentinel/"
$SUDO mkdir -p "/var/lib/sentinel/"

if [ -f "$PROYECTO_DIR/config/sentinel.json" ]; then
  if [ ! -f "/etc/sentinel/sentinel.json" ]; then
    $SUDO cp "$PROYECTO_DIR/config/sentinel.json" "/etc/sentinel/sentinel.json"
    $SUDO chmod 600 "/etc/sentinel/sentinel.json"
  fi
else
  echo "Error: no se encontro el archivo de configuraciones"
  exit 1
fi

if [ -f "$PROYECTO_DIR/scripts/sentinel-cli.sh" ]; then
    $SUDO cp "$PROYECTO_DIR/scripts/sentinel-cli.sh" "/usr/local/bin/sentinel-cli"
    $SUDO chmod +x /usr/local/bin/sentinel-cli
    echo "Se copio el scrips cliente correctamente"
else
    echo "No se encontro el cliente sentinel-cli.sh"
    exit 1
fi

if [ -z "$DISPLAY" ] && [ -z "$WAYLAND_DISPLAY" ]; then
  echo "No se detectó entorno gráfico, instalando en modo servidor"

  if [ -f "/etc/systemd/system/sentinel.service" ]; then
      $SUDO systemctl disable sentinel.service || true
      $SUDO rm -f "/etc/systemd/system/sentinel.service"
      $SUDO systemctl reset-failed sentinel.service || true
      $SUDO systemctl daemon-reload
      echo "Se limpió el servicio anterior (modo servidor)"
  fi

  if ! id "$NOMBRE_USUARIO" &> /dev/null; then
    $SUDO useradd --system --no-create-home --shell /usr/sbin/nologin "$NOMBRE_USUARIO"
    echo "Usuario de sistema '$NOMBRE_USUARIO' creado"
  fi

  $SUDO chown -R "$NOMBRE_USUARIO:$NOMBRE_USUARIO" /etc/sentinel /var/log/sentinel /var/lib/sentinel /usr/local/bin/sentinel /usr/local/bin/sentinel-cli


      $SUDO bash -c "cat > /etc/systemd/system/sentinel.service" <<EOF
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

$SUDO systemctl daemon-reload
$SUDO systemctl enable sentinel.service
$SUDO systemctl start sentinel.service
$SUDO systemctl restart sentinel.service
$SUDO systemctl status sentinel.service

else
  echo "Entorno gráfico detectado, instalando en modo escritorio"

  if [ -f "$HOME/.config/systemd/user/sentinel.service" ]; then
      systemctl --user disable sentinel.service || true
      rm -f "$HOME/.config/systemd/user/sentinel.service"
      systemctl --user reset-failed sentinel.service || true
      systemctl --user daemon-reload
      echo "Se limpió el servicio anterior"
  fi

  if [ -n "$1" ]; then
      echo "Aviso: el argumento de usuario '$1' se ignora en modo escritorio (el servicio corre como tu usuario actual: $USER)"
  fi
  $SUDO chown -R "$USER:$USER" /etc/sentinel /var/log/sentinel /var/lib/sentinel /usr/local/bin/sentinel /usr/local/bin/sentinel-cli

# CONFIGURACIÓN DEL NUEVO SERVICIO SYSTEMD
echo "Configurando service..."
DIR_SERVICIOS_USER="$HOME/.config/systemd/user"
mkdir -p "$DIR_SERVICIOS_USER"

cat > "$DIR_SERVICIOS_USER/sentinel.service" << EOF
[Unit]
Description=Daemon Sentinel
After=graphical-session.target

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

[Install]
WantedBy=graphical-session.target
EOF

systemctl --user daemon-reload
systemctl --user enable sentinel.service
systemctl --user start sentinel.service
systemctl --user status sentinel.service
fi

echo "Instalación completa de manera exitosa"