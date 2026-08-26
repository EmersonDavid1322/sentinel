#!/bin/bash
set -e

if [ "$EUID" -eq 0 ]; then
    SUDO=""
else
    SUDO="sudo"
fi

BASE_DIR=$(dirname "$(readlink -f "$0")")
PROYECTO_DIR=$(dirname "$BASE_DIR")

MODO_INSTALACION=""
NOMBRE_USUARIO=""

# 1. Procesar argumentos de forma correcta
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --server)
            MODO_INSTALACION="server"
            # Si el siguiente argumento existe y no empieza con "-", es el usuario
            if [[ -n "$2" && "$2" != -* ]]; then NOMBRE_USUARIO="$2"; shift; else NOMBRE_USUARIO="sentinel"; fi
            ;;
        --desktop)
            MODO_INSTALACION="desktop"
            # Si el siguiente argumento existe y no empieza con "-", es el usuario. Si no, usa el $USER actual
            if [[ -n "$2" && "$2" != -* ]]; then NOMBRE_USUARIO="$2"; shift; else NOMBRE_USUARIO="$USER"; fi
            ;;
        *)
            echo "Parámetro desconocido: $1"
            exit 1
            ;;
    esac
    shift
done

if [ -z "$MODO_INSTALACION" ]; then
    echo "Error: Debes especificar un modo de instalación."
    echo "Uso: $0 --server [usuario]  O  $0 --desktop [usuario]"
    exit 1
fi

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

if [ -f "$PROYECTO_DIR/build/sentinel" ]; then
  echo "Binario detectado"
else
  echo "No se a compilado el binario"
  exit 1
fi

if [ -f "/usr/local/bin/sentinel" ]; then
    echo "Eliminando versión de ejecutable anterior..."
    $SUDO rm "/usr/local/bin/sentinel"
fi

$SUDO cp "$PROYECTO_DIR/build/sentinel" "/usr/local/bin/"

$SUDO chmod +x /usr/local/bin/sentinel

# Limpieza de la carpeta temporal de compilación
cd "$PROYECTO_DIR"

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

if [ "$MODO_INSTALACION" == "server" ]; then
    echo "Iniciando configuración en Modo Servidor para el usuario: $NOMBRE_USUARIO"
    $SUDO bash "$PROYECTO_DIR/scripts/setup_server.sh" "$NOMBRE_USUARIO"
else
    echo "Iniciando configuración en Modo Escritorio para el usuario: $NOMBRE_USUARIO"
    $SUDO bash "$PROYECTO_DIR/scripts/setup_desktop.sh" "$NOMBRE_USUARIO"
fi

echo "¡Instalación completa de manera exitosa en modo $MODO_INSTALACION!"