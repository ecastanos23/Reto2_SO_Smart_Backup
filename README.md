# Smart Backup Kernel-Space Utility

Este proyecto implementa y compara dos métodos de copia de archivos a nivel de sistema para analizar el impacto del "Context Switch" (cambio de contexto) en el rendimiento del procesador. Además, ahora soporta dos modos opcionales de compresión en formato gzip para evaluar la eficiencia total del flujo de backup.

## 1. Descripción Técnica

El sistema contrasta dos enfoques:
* **Capa de Kernel (`sys_smart_copy`):** Utiliza llamadas directas al sistema (`open`, `read`, `write`) con un buffer optimizado al tamaño de una página de memoria (4096 bytes). Al agrupar las peticiones, minimiza los cambios entre el Modo Usuario y el Modo Kernel, maximizando el *throughput*.
* **Capa de Usuario (`lib_standard_copy`):** Emplea la librería estándar de C (`fread`, `fwrite`). Delega la gestión de memoria al espacio de usuario, aprovechando el "buffering" interno de la librería.

### 1.1. Flujos de Compresión
Para analizar el impacto en I/O, el sistema implementa:
1. **Backup simple:** Copia estándar.
2. **Backup y luego compresión (`--compress-after`):** Escribe el respaldo completo en disco y luego lo comprime.
3. **Compresión y luego backup (`--compress-first`):** Comprime los datos en un temporal antes de copiarlos.

## 2. Arquitectura del Proyecto

```text
smart_backup_project/
├── Makefile                  # Reglas de compilación automatizada
├── include/
│   └── smart_copy.h          # Firmas y constantes (Buffer = 4096)
├── src/
│   ├── backup_engine.c       # Motores de copia (Syscall vs stdio) y lógica gzip
│   └── main.c                # Interfaz, CLI, orquestación y medición de tiempo
├── tests/                    # Carpeta para archivos de prueba
└── docs/
    └── reporte.pdf           # Análisis técnico y tabla comparativa
```

## 3. Requisitos Previos
- Entorno Linux/Unix.
- Compilador GCC.
- Herramienta make.
- Librería zlib (para compresión gzip).

En Ubuntu/Debian, instale las dependencias con:
```bash
sudo apt-get update
sudo apt-get install -y build-essential zlib1g-dev
```

## 4. Instrucciones de Compilación
Para compilar el proyecto utilizando las banderas de optimización (-O2) y advertencia (-Wall -Wextra), ejecute en la raíz del proyecto:

```bash
make clean
make
```

Esto generará el ejecutable `smart_backup`. Para limpiar los archivos binarios compilados (objetos, ejecutable y archivos de la carpeta tests), ejecute:
```bash
make clean
```

## 5. Generación de Archivos de Prueba (Rendimiento)
Para evaluar el sistema bajo diferentes cargas, se usa el comando dd de Linux para generar archivos binarios exactos leyendo ceros desde /dev/zero.

### A. Crear archivo de 1 KB:
```bash
mkdir -p tests
dd if=/dev/zero of=tests/test_1KB.bin bs=1K count=1
```

### B. Crear archivo de 1 MB:
```bash
dd if=/dev/zero of=tests/test_1MB.bin bs=1M count=1
```

### C. Crear archivo masivo de 1 GB:
```bash
dd if=/dev/zero of=tests/test_1GB.bin bs=1M count=1024
```
Se implementó `bs=1M` (tamaño de bloque de 1 Megabyte) y `count=1024` (escribir 1024 bloques) para evitar saturar la memoria RAM generando el Gigabyte en una sola pasada.

## 6. Ejecución de Pruebas (Backup Simple)
Una vez compilado el código y generados los archivos de prueba, ejecute el programa pasando la ruta del archivo de origen y las dos rutas de destino:

### Para 1 KB
```bash
./smart_backup tests/test_1KB.bin tests/sys_base_1KB.bin tests/lib_base_1KB.bin
```

### Para 1 MB
```bash
./smart_backup tests/test_1MB.bin tests/sys_base_1MB.bin tests/lib_base_1MB.bin
```

### Para 1 GB
```bash
./smart_backup tests/test_1GB.bin tests/sys_base_1GB.bin tests/lib_base_1GB.bin
```

### Prueba de Error
Si el programa no encuentra el archivo, no debe colapsar, sino realizar el manejo de error:
```bash
./smart_backup tests/test_GB.bin tests/sys_GB.bin tests/lib_GB.bin
```
El programa imprimirá en consola el tiempo exacto en segundos que tomó cada método utilizando el reloj monotónico del procesador (`CLOCK_MONOTONIC`).

## 7. Ejecución de Pruebas Avanzadas (Compresión)
Permite comparar la eficiencia del flujo de backup cuando se involucra compresión. *(Asegúrese de usar rutas reales o generadas previamente)*.

### A. Modo Backup y luego Compresión
Se crean respaldos normales y luego se comprime cada uno (`.gz`).
```bash
./smart_backup --compress-after tests/test_1KB.bin tests/sys_after_1KB.bin tests/lib_after_1KB.bin
```
**Validación:** Los `.bin` deben coincidir con el origen.
```bash
cmp -s tests/test_1KB.bin tests/sys_after_1KB.bin && echo "sys_after OK" || echo "sys_after FAIL"
cmp -s tests/test_1KB.bin tests/lib_after_1KB.bin && echo "lib_after OK" || echo "lib_after FAIL"
```

### B. Modo Compresión y luego Backup
Se comprime el origen en un temporal, se hace el backup de los datos comprimidos y se elimina el temporal.
```bash
./smart_backup --compress-first tests/test_1KB.bin tests/sys_first_1KB.bin tests/lib_first_1KB.bin
```
**Validación:** Los archivos resultantes son nativamente formato gzip.
```bash
file tests/sys_first_1KB.bin tests/lib_first_1KB.bin
```

## 8. Análisis e Interpretación de Eficiencia
En la salida del programa, compare las líneas:
* `[Resumen] Tiempo total backup->compresion: ...`
* `[Resumen] Tiempo total compresion->backup: ...`

**Interpretación:**
* `--compress-after`: Conserva la copia sin comprimir y luego genera la comprimida. Es más simple para restauración directa del `.bin`, pero escribe más datos en disco y requiere más I/O.
* `--compress-first`: Reduce el I/O total porque respalda directamente datos comprimidos. Suele ser más eficiente en tiempo/espacio para archivos grandes.

*Nota:* Para medir la eficiencia real, compare con archivos de 10MB o mayores. Con 1KB, el ruido del sistema puede dominar la medición.
