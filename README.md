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
sentinel-cli backup activar 
```
```bash
sentinel-cli organizador desactivar
```

**Comandos Backup**

- Añadir alguna carpeta a la lista de carpetas del backup
```bash
sentinel-cli backup añadir_carpeta dirección
```

- Modificar el destino del backup
```bash
sentinel-cli backup destino dirección
```

- Ejecutar un backup
```bash
sentinel-cli backup ahora
```

**Comandos Monitor**

- Cambiar el valor limite del monitor: limite_cpu, limite_ram, limite_disco
```bash
sentinel-cli monitor limite_cpu 90
```

- Ejecutar un monitoreo con el uso CPU, RAM y disco actual
```bash
 sentinel-cli monitor consumo
```

**Comandos Organizador**

- Cambiar dirrción de la carpeta vigilada
```bash
sentinel-cli organizador carpeta_vigilar dirección
```

- Agregar una regla al organizador
```bash
sentinel-cli organizador agregar_regla 'extension|dirección_destino'
```

**Comandos Estado**

- Ver la configuración actual de todos los modulos
```bash
sentinel-cli estado
```

- Ver la configuración actual de algun modulo
```bash
sentinel-cli estado backup_local
```
```bash
sentinel-cli estado monitor
```
```bash
sentinel-cli estado organizador
```

## Requisitos

- Linux
- g++ con soporte C++17
- CMake 3.10+
- systemd

## Instalación
-**Versión Escritorio**
```bash
git clone https://github.com/EmersonDavid1322/sentinel
cd sentinel
cmake -S . -B build
cmake --build build
bash scripts/install.sh --desktop [USUARIO]
```

-**Versión sin entorno grafico o servidor**
```bash
git clone https://github.com/EmersonDavid1322/sentinel
cd sentinel
cmake -S . -B build
cmake --build build
bash scripts/install.sh --server [USUARIO OPCIONAL]
```

## Estado

🚧 En desarrollo activo
