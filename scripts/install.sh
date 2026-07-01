#!/bin/bash
set -e
BASE_DIR=$(dirname "$(readlink -f "$0")")
PROYECTO_DIR=$(dirname "$BASE_DIR")
DESTINO_DEAMON="$HOME/apps/deamon"

mkdir -p "$DESTINO_DEAMON"

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

if ! command -v g++ &> /dev/null; then
    echo "g++ no encontrado. Intalando..."
    $INSTALAR g++ build-essential
fi

if ! command -v cmake &> /dev/null; then
    echo "Cmake no encontrado. Intalando..."
    $INSTALAR cmake
fi

echo "Compilado Sentinel..."
mkdir -p "$PROYECTO_DIR/build"
cd "$PROYECTO_DIR/build"
cmake "$PROYECTO_DIR" -DCMAKE_BUILD_TYPE=Release
make

mkdir -p "$DESTINO_DEAMON/config"

if [ -f "$DESTINO_DEAMON/sentinel" ]; then
    echo "Elimando version anterior..."
    rm "$DESTINO_DEAMON/sentinel"
fi

cp "$PROYECTO_DIR/build/sentinel" "$DESTINO_DEAMON/"
if [ ! -f "$DESTINO_DEAMON/config/sentinel.json" ]; then
    cp "$PROYECTO_DIR/config/sentinel.json" "$DESTINO_DEAMON/config/"
fi

chmod +x "$DESTINO_DEAMON/sentinel"

rm -rf "$BASE_DIR/build"

if [ -f "$HOME/.config/systemd/user/sentinel.service" ]; then
    systemctl --user disable sentinel.service
    rm -f "$HOME/.config/systemd/user/sentinel.service"
    systemctl --user daemon-reload
    echo "Se limpio el servicio anteriro"
fi

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

echo "Instalación completa"