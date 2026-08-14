#!/bin/bash
set -e

RUTA_FIFO="/var/lib/sentinel/sentinel.fifo"
RUTA_ESTADO="/var/lib/sentinel/sentinel_estado.txt"

TIEMPO_ANTES=$(stat -c %Y "$RUTA_ESTADO" 2>/dev/null || echo 0)

echo "$@" > "$RUTA_FIFO"

for i in $(seq 1 9000); do
    TIEMPO_DESPUES=$(stat -c %Y "$RUTA_ESTADO" 2>/dev/null || echo 0)
    if [ "$TIEMPO_DESPUES" != "$TIEMPO_ANTES" ]; then
        break
    fi
    sleep 0.1
done

cat "$RUTA_ESTADO"