#!/bin/bash
set -e

BASE_DIR=$(dirname "$(readlink -f "$0")")
RUTA_FIFO="$BASE_DIR/config/sentinel.fifo"
RUTA_ESTADO="$BASE_DIR/config/sentinel_estado.txt"

echo "$@" > "$RUTA_FIFO"
sleep 0.5
cat "$RUTA_ESTADO"