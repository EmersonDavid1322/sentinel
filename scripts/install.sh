#!/bin/bash
set -e

BASE_DIR=$(dirname "$(readlink -f "$0")")
PROYECTO_DIR=$(dirname "$BASE_DIR")
DESTINO_DEAMON="$HOME/apps/deamon"

mkdir -p "$DESTINO_DEAMON"

# DETECCIÓN DE ENTORNO Y GESTOR DE PAQUETES
if command -v apt &> /dev/null; then 
    INSTALAR="sudo apt install -y"
elif command -v dnf &> /dev/null; then 
    INSTALAR="sudo dnf install -y"
elif command -v pacman &> /dev/null; then
    INSTALAR="sudo pacman -S --noconfirm"
else 
    echo "Gestor de paquetes no soportado"
    exit 1
fi

# INSTALACIÓN DE DEPENDENCIAS
if ! command -v g++ &> /dev/null; then
    echo "g++ no encontrado. Instalando..."
    $INSTALAR g++ build-essential
fi

if ! command -v cmake &> /dev/null; then
    echo "Cmake no encontrado. Instalando..."
    $INSTALAR cmake
fi

# limpieza
echo "Deteniendo servicios e instancias previas de Sentinel..."

if systemctl --user is-active --quiet sentinel.service 2>/dev/null; then
    systemctl --user stop sentinel.service || true
fi

# Deshabilitar y eliminar el archivo de servicio viejo
if [ -f "$HOME/.config/systemd/user/sentinel.service" ]; then
    systemctl --user disable sentinel.service || true
    rm -f "$HOME/.config/systemd/user/sentinel.service"
    systemctl --user reset-failed sentinel.service || true
    systemctl --user daemon-reload
    echo "Se limpió el servicio anterior"
fi

# Por si el proceso quedó 'huérfano' suelto en la memoria (fuera de systemd)
if pgrep -x "sentinel" > /dev/null; then
    echo "Detectado proceso huérfano en ejecución. Enviando SIGTERM..."
    pkill -15 -x "sentinel" || true
    
    # Damos 3 segundos a tus hilos de C++ para que terminen sus .join() limpiamente
    sleep 3 
    
    # Si sigue vivo de forma terca, lo forzamos a cerrar
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

mkdir -p "$DESTINO_DEAMON/config"

if [ -f "$DESTINO_DEAMON/sentinel" ]; then
    echo "Eliminando versión de ejecutable anterior..."
    rm "$DESTINO_DEAMON/sentinel"
fi

cp "$PROYECTO_DIR/build/sentinel" "$DESTINO_DEAMON/"
if [ ! -f "$DESTINO_DEAMON/config/sentinel.json" ]; then
    cp "$PROYECTO_DIR/config/sentinel.json" "$DESTINO_DEAMON/config/"
fi

chmod +x "$DESTINO_DEAMON/sentinel"

# Limpieza de la carpeta temporal de compilación
rm -rf "$PROYECTO_DIR/build"

# CONFIGURACIÓN DEL NUEVO SERVICIO SYSTEMD
echo "Configurando service..."
DIR_SERVICIOS_USER="$HOME/.config/systemd/user"
mkdir -p "$DIR_SERVICIOS_USER"

cat > "$DIR_SERVICIOS_USER/sentinel.service" <<EOF
[Unit]
Description=Daemon Sentinel
After=graphical-session.target

[Service]
ExecStart=$DESTINO_DEAMON/sentinel
WorkingDirectory=$DESTINO_DEAMON
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

echo "Instalación completa de manera exitosa"


if [ -f "$PROYECTO_DIR/scripts/sentinel-cli.sh" ]; then
    cp "$PROYECTO_DIR/scripts/sentinel-cli.sh" "$DESTINO_DEAMON"
    chmod +x "$DESTINO_DEAMON/sentinel-cli.sh"
    echo "Se copio el scrips cliente correctamente"
else
    echo "No se encontro el cliente sentinel-cli.sh"
fi