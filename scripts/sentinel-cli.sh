#!/bin/bash
set -e

RUTA_SOCKET="/var/lib/sentinel/sentinel.sock"

echo "$@" | socat - UNIX-CONNECT:"$RUTA_SOCKET"