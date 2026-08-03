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

- Posibilidad de configurar por `config/sentinel.json`
- Se puede reconfigurar mientras se ejecuta.
- Opción de forzar backup si no hay suficiente espacio o el consumo de cpu actual supera 
los limites impuestos.

## Comandos Via FIFO
**Se recomienda el uso del script 'sentinel-cli.sh'**

Utilizar comandos para controlar el Sentinel:

**Comando activar/desactivar módulos**
```bash
bash sentinel-cli.sh backup_local activar 
```
```bash
bash sentinel-cli.sh organizador desactivar
```

**Comandos Backup**

- Añadir alguna carpeta a la lista de carpetas del backup
```bash
bash sentinel-cli.sh backup_local añadir_carpeta dirección
```

- Modificar el destino del backup
```bash
bash sentinel-cli.sh backup_local destino dirección
```

- Ejecutar un backup
```bash
bash sentinel-cli.sh backup_local ahora
```

**Comandos Monitor**

- Cambiar el valor limite del monitor: limite_cpu, limite_ram, limite_disco
```bash
bash sentinel-cli.sh monitor limite_cpu 90
```

- Ejecutar un monitoreo con el uso CPU, RAM y disco actual
```bash
bash sentinel-cli.sh monitor consumo
```

**Comandos Organizador**

- Cambiar dirrción de la carpeta vigilada
```bash
bash sentinel-cli.sh organizador carpeta_vigilar dirección
```

- Agregar una regla al organizador
```bash
bash sentinel-cli.sh organizador agregar_regla 'extension|dirección_destino'
```

**Comandos Estado**

- Ver la configuración actual de todos los modulos
```bash
bash sentinel-cli.sh estado
```

- Ver la configuración actual de algun modulo
```bash
bash sentinel-cli.sh estado backup_local
```
```bash
bash sentinel-cli.sh estado monitor
```
```bash
bash sentinel-cli.sh estado organizador
```

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
