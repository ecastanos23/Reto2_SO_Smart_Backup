# Smart Backup Project

Proyecto en C para comparar estrategias de respaldo de archivos y su rendimiento.

El programa ejecuta dos metodos de copia:
- Copia tipo syscall (`open/read/write`)
- Copia tipo libreria estandar (`fopen/fread/fwrite`)

Ademas, ahora soporta dos modos opcionales de compresion en formato gzip para evaluar eficiencia total del flujo de backup.

## Objetivo

Comparar tiempo y comportamiento de:
1. Backup simple
2. Backup y luego compresion (`--compress-after`)
3. Compresion y luego backup (`--compress-first`)

## Estructura

- `include/smart_copy.h`: firmas y `BUFFER_SIZE`
- `src/backup_engine.c`: logica de copia y compresion gzip
- `src/main.c`: CLI, medicion de tiempos y orquestacion
- `Makefile`: compilacion y limpieza
- `tests/`: archivos de prueba generados

## Requisitos

- GCC
- Make
- zlib (para compresion gzip)

En Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install -y build-essential zlib1g-dev
```

## Compilacion

Desde `smart_backup_project`:

```bash
make clean
make
```

Se genera el ejecutable `smart_backup`.

## Uso

Importante:
- Los textos entre `<` y `>` son placeholders. No se escriben literalmente en la terminal.
- Ejemplo: en lugar de `./smart_backup <origen> <destino_sys> <destino_lib>`, usa rutas reales.

## Paso a paso de prueba (recomendado)

### 1) Entrar al proyecto y compilar

```bash
cd /workspaces/Reto2_SO_Smart_Backup/smart_backup_project
make clean
make
```

### 2) Crear archivo de prueba (1KB)

```bash
mkdir -p tests
dd if=/dev/zero of=tests/test_1KB.bin bs=1K count=1
```

### 3) Probar modo original (backup simple)

```bash
./smart_backup tests/test_1KB.bin tests/sys_base_1KB.bin tests/lib_base_1KB.bin
```

### 4) Probar modo `--compress-after` (backup y luego compresion)

```bash
./smart_backup --compress-after tests/test_1KB.bin tests/sys_after_1KB.bin tests/lib_after_1KB.bin
```

Verificar que existen:
- `tests/sys_after_1KB.bin`
- `tests/lib_after_1KB.bin`
- `tests/sys_after_1KB.bin.gz`
- `tests/lib_after_1KB.bin.gz`

### 5) Probar modo `--compress-first` (compresion y luego backup)

```bash
./smart_backup --compress-first tests/test_1KB.bin tests/sys_first_1KB.bin tests/lib_first_1KB.bin
```

Verificar que existen:
- `tests/sys_first_1KB.bin`
- `tests/lib_first_1KB.bin`

### 6) Validar resultados

```bash
# En compress-after, los .bin deben coincidir con el origen
cmp -s tests/test_1KB.bin tests/sys_after_1KB.bin && echo "sys_after OK" || echo "sys_after FAIL"
cmp -s tests/test_1KB.bin tests/lib_after_1KB.bin && echo "lib_after OK" || echo "lib_after FAIL"

# En compress-first, los respaldos son gzip
file tests/sys_first_1KB.bin tests/lib_first_1KB.bin
```

### 7) Comparar eficiencia

En la salida del programa, comparar las lineas:
- `[Resumen] Tiempo total backup->compresion: ...`
- `[Resumen] Tiempo total compresion->backup: ...`

El menor valor corresponde al flujo mas eficiente para ese archivo.

### 1) Backup simple (modo original)

```bash
./smart_backup <origen> <destino_sys> <destino_lib>
```

Salida esperada:
- Tiempo de copia syscall
- Tiempo de copia libreria
- Tiempo total de backup simple

### 2) Backup y luego compresion

```bash
./smart_backup --compress-after <origen> <destino_sys> <destino_lib>
```

Flujo:
1. Se crean respaldos normales: `<destino_sys>` y `<destino_lib>`
2. Se comprime cada respaldo y se generan:
   - `<destino_sys>.gz`
   - `<destino_lib>.gz`
3. Se imprime tiempo total del flujo backup->compresion

### 3) Compresion y luego backup

```bash
./smart_backup --compress-first <origen> <destino_sys> <destino_lib>
```

Flujo:
1. Se comprime el archivo origen a un temporal `.backup_tmp.gz`
2. El backup syscall y libreria copia ese contenido comprimido
3. Los archivos de salida (`<destino_sys>`, `<destino_lib>`) quedan en formato gzip
4. Se elimina el temporal al finalizar
5. Se imprime tiempo total del flujo compresion->backup

## Ejemplo rapido (1KB)

```bash
mkdir -p tests
dd if=/dev/zero of=tests/test_1KB.bin bs=1K count=1

./smart_backup --compress-after tests/test_1KB.bin tests/sys_after_1KB.bin tests/lib_after_1KB.bin
./smart_backup --compress-first tests/test_1KB.bin tests/sys_first_1KB.bin tests/lib_first_1KB.bin
```

Validaciones sugeridas:

```bash
# En compress-after, los .bin deben coincidir con el origen
cmp -s tests/test_1KB.bin tests/sys_after_1KB.bin && echo "sys_after OK"
cmp -s tests/test_1KB.bin tests/lib_after_1KB.bin && echo "lib_after OK"

# En compress-first, los respaldos son gzip
file tests/sys_first_1KB.bin tests/lib_first_1KB.bin
```

## Interpretacion de eficiencia

- `--compress-after`: conserva copia sin comprimir y luego genera comprimida. Es mas simple para restauracion directa del .bin, pero escribe mas datos en disco.
- `--compress-first`: reduce I/O total porque respalda directamente datos comprimidos. Suele ser mas eficiente en tiempo/espacio para archivos grandes.

Nota: para medir eficiencia real, comparar con archivos de 10MB, 100MB o mayores. Con 1KB el ruido del sistema puede dominar la medicion.

## Limpieza

```bash
make clean
```

El objetivo `clean` elimina:
- objetos (`src/*.o`)
- ejecutable (`smart_backup`)
- archivos de prueba en `tests/` (`*.bin`, `*.gz`, `*_copy.bin`)
