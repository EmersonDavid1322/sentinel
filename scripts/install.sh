#!/bin/bash
set -e
BASE_DIR=$(dirname "$(readlink -f "$0")")
CURRENT_USER=$(whoami)
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
mkdir -p "$BASE_DIR/build"
cd "$BASE_DIR/build"
cmake .. -DCMAKE_BUILD_TYPE=Release
make

mkdir -p "$DESTINO_DEAMON/config"
cp "$BASE_DIR/build/sentinel" "$DESTINO_DEAMON/"
if [ ! -f "$DESTINO_DEAMON/config/sentinel.json" ]; then
    cp "$BASE_DIR/config/sentinel.json" "$DESTINO_DEAMON/config/"
fi

chmod +x "$DESTINO_DEAMON/sentinel"

rm -rf "$BASE_DIR/build"

echo "Configurando service..."
DIR_SERVICIOS_USER="$HOME/.config/systemd/user/"
mkdir -p "$DIR_SERVICIOS"

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