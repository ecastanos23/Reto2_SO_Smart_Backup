# Smart Backup Kernel-Space Utility

Este proyecto implementa y compara dos métodos de copia de archivos a nivel de sistema para analizar el impacto del "Context Switch" (cambio de contexto) en el rendimiento del procesador.

## 1. Descripción Técnica

El sistema contrasta dos enfoques:
* **Capa de Kernel (`sys_smart_copy`):** Utiliza llamadas directas al sistema (`open`, `read`, `write`) con un buffer optimizado al tamaño de una página de memoria (4096 bytes). Al agrupar las peticiones, minimiza los cambios entre el Modo Usuario y el Modo Kernel, maximizando el *throughput*.
* **Capa de Usuario (`lib_standard_copy`):** Emplea la librería estándar de C (`fread`, `fwrite`). Delega la gestión de memoria al espacio de usuario, aprovechando el "buffering" interno de la librería.

## 2. Arquitectura del Proyecto

```text
smart_backup_project/
├── Makefile                  # Reglas de compilación automatizada
├── include/
│   └── smart_copy.h          # Firmas y constantes (Buffer = 4096)
├── src/
│   ├── backup_engine.c       # Motores de copia (Syscall vs stdio)
│   └── main.c                # Interfaz, validaciones y medición de tiempo
├── tests/                    # Carpeta para archivos de prueba
└── docs/
    └── reporte.pdf           # Análisis técnico y tabla comparativa
```
## 3. Requisitos Previos
- Entorno Linux/Unix.
- Compilador GCC.
- Herramienta make.

## 4. Instrucciones de Compilación
Para compilar el proyecto utilizando las banderas de optimización (-O2) y advertencia (-Wall -Wextra), ejecute en la raíz del proyecto:

``` Bash
make
```

Esto generará el ejecutable smart_backup. Para limpiar los archivos binarios compilados, ejecute:
``` Bash
make clean
```

## 5. Generación de Archivos de Prueba (Rendimiento)
Para evaluar el sistema bajo diferentes cargas, se usa el comando dd de Linux para generar archivos binarios exactos leyendo ceros desde /dev/zero.

### A. Crear archivo de 1 KB:
``` Bash
dd if=/dev/zero of=tests/test_1KB.bin bs=1K count=1
```

### B. Crear archivo de 1 MB:

``` Bash
dd if=/dev/zero of=tests/test_1MB.bin bs=1M count=1
```

### C. Crear archivo masivo de 1 GB:

``` Bash
dd if=/dev/zero of=tests/test_1GB.bin bs=1M count=1024
```

Se implementó bs=1M (tamaño de bloque de 1 Megabyte) y count=1024 (escribir 1024 bloques) para evitar saturar la memoria RAM generando el Gigabyte en una sola pasada.

## 6. Ejecución de Pruebas
Una vez compilado el código y generados los archivos de prueba, ejecute el programa pasando la ruta del archivo de origen y las dos rutas de destino:

### Para 1MB

``` Bash
./smart_backup tests/test_1MB.bin tests/sys_1MB.bin tests/lib_1MB.bin
```

### Para 1GB

``` Bash
./smart_backup tests/test_1GB.bin tests/sys_1GB.bin tests/lib_1GB.bin
```
El programa imprimirá en consola el tiempo exacto en segundos que tomó cada método utilizando el reloj monotónico del procesador (CLOCK_MONOTONIC).

