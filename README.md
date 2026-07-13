# Sentinel

Daemon de sistema para Linux escrito en C++.
Corre en segundo plano con systemd y automatiza tareas de mantenimiento del sistema.

## Módulos

### Backup
- Respalda carpetas configurables automáticamente
- Programable por hora fija
- Registro de cada backup realizado
- Notificación al completar o si falla

### Monitor
- Vigila uso de CPU, RAM y disco
- Alertas configurables por porcentaje
- Log del historial del sistema

### Organizador
- Vigila carpetas configuradas
- Mueve archivos automáticamente según su extensión
- Reglas configurables por el usuario

## Configuración

Todo se configura `config/sentinel.json` sin necesidad de recompilar.

## Requisitos

- Linux
- g++ con soporte C++17
- CMake 3.10+
- systemd

## Instalación
```bash
git clone https://github.com/EmersonDavid1322/sentinel
cd sentinel
bash scripts/install.sh
```

## Estado

🚧 En desarrollo activo
