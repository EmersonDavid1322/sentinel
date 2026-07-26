#!/bin/bash
set -e

BASE_DIR=$(dirname "$(readlink -f "$0")")
RUTA_FIFO="$BASE_DIR/config/sentinel.fifo"
RUTA_ESTADO="$BASE_DIR/config/sentinel_estado.txt"

TIEMPO_ANTES=$(stat -c %Y "$RUTA_ESTADO" 2>/dev/null || echo 0)

echo "$@" > "$RUTA_FIFO"

for i in $(seq 1 300); do
    TIEMPO_DESPUES=$(stat -c %Y "$RUTA_ESTADO" 2>/dev/null || echo 0)
    if [ "$TIEMPO_DESPUES" != "$TIEMPO_ANTES" ]; then
        break
    fi
    sleep 0.1
done

cat "$RUTA_ESTADO"