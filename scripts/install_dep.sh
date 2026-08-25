#!/bin/bash

# 1. Definir listas de paquetes corregidas según la distribución
DEBIAN_PKGS=("pkg-config" "libcurl4-openssl-dev" "libnotify-dev" "g++" "build-essential" "cmake")
FEDORA_PKGS=("pkgconf-pkg-config" "libcurl-devel" "libnotify-devel" "gcc-c++" "make" "cmake")
ARCH_PKGS=("pkgconf" "curl" "libnotify" "gcc" "base-devel" "cmake")

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

# 3. Validar privilegios de root o sudo
if [ "$EUID" -eq 0 ]; then
    PREFIX=""
elif command -v sudo &> /dev/null; then
    PREFIX="sudo"
else
    echo "❌ Error: Este script requiere privilegios de root o 'sudo' instalado."
    exit 1
fi

echo "⏳ Instalando dependencias..."
$PREFIX $COMANDO_INSTALACION "${PAQUETES[@]}"