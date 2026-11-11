#!/bin/bash
set -e

RUTA_SOCKET="/var/lib/sentinel/sentinel.sock"
RUTA_ESTADO="/var/lib/sentinel/sentinel_estado.txt"

TIEMPO_ANTES=$(stat -c %Y "$RUTA_ESTADO" 2>/dev/null || echo 0)

echo "$@" | socat - UNIX-CONNECT:"$RUTA_SOCKET"

for i in $(seq 1 9000); do
    TIEMPO_DESPUES=$(stat -c %Y "$RUTA_ESTADO" 2>/dev/null || echo 0)
    if [ "$TIEMPO_DESPUES" != "$TIEMPO_ANTES" ]; then
        break
    fi
    sleep 0.1
done

cat "$RUTA_ESTADO"