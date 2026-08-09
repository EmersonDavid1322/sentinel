#!/bin/bash

# 1. Definir listas de paquetes según la distribución
DEBIAN_PKGS=("pkg-config" "libcurl4-openssl-dev" "libnotify-dev" "g++ build-essential")
FEDORA_PKGS=("pkgconf-pkg-config" "libcurl-devel" "libnotify-devel" "g++ build-essential")
ARCH_PKGS=("pkgconf" "curl" "libnotify" "g++ build-essential")

# 2. Detectar el gestor de paquetes del sistema
if command -v apt &> /dev/null; then
    PM="apt"
    PAQUETES=("${DEBIAN_PKGS[@]}")
    COMANDO_INSTALACION="apt install -y"
elif command -v dnf &> /dev/null; then
    PM="dnf"
    PAQUETES=("${FEDORA_PKGS[@]}")
    COMANDO_INSTALACION="dnf install -y"
elif command -v pacman &> /dev/null; then
    PM="pacman"
    PAQUETES=("${ARCH_PKGS[@]}")
    COMANDO_INSTALACION="pacman -S --noconfirm"
else
    echo "❌ Error: Gestor de paquetes no soportado."
    exit 1
fi

echo "📦 Detectado gestor: $PM. Preparando instalación..."

if command -v sudo &> /dev/null && [ "$EUID" -ne 0 ]; then
    PREFIX="sudo"
else
    PREFIX=""
fi

echo "⏳ Instalando dependencias: ${PAQUETES[*]}..."
eval "$PREFIX $COMANDO_INSTALACION ${PAQUETES[*]}"

if [ $? -eq 0 ]; then
    echo "✅ ¡Todas las dependencias se instalaron correctamente!"
else
    echo "❌ Hubo un error al instalar las dependencias."
    exit 1
fi
