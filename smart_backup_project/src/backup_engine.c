// Antes de copiar, se debe pedirle permiso 
// al sistema operativo para acceder al origen 
// y preparar el destino.

#include "smart_copy.h"
#include <fcntl.h>   // Para las banderas O_RDONLY, O_WRONLY, etc.
#include <unistd.h>  // Para close(), read(), write()
#include <stdio.h>   // Para perror()
#include <zlib.h>    // Para gzopen(), gzwrite(), gzclose()

int sys_smart_copy(const char *src, const char *dest) {
    // 1. Abrir origen
    int fd_in = open(src, O_RDONLY);
    if (fd_in < 0) {
        perror("Error al abrir el archivo de origen");
        return -1;
    }

    // 2. Abrir destino
    int fd_out = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out < 0) {
        perror("Error al crear el archivo de destino");
        // Advertencia: Si falla aquí, se debe cerrar el origen para no dejar recursos colgados (fugas de memoria/descriptores).
        close(fd_in); 
        return -1;
    }
    
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    ssize_t bytes_written;

    // El ciclo continúa mientras read() devuelva un número mayor a 0 (es decir, mientras haya datos).
    // read() intentará llenar los 4096 bytes del buffer en cada "bombeo".
    while ((bytes_read = read(fd_in, buffer, BUFFER_SIZE)) > 0) {
        // Escribir exactamente la cantidad de bytes que logramos leer.
        bytes_written = write(fd_out, buffer, bytes_read);
        // Advertencia técnica: Validar discrepancias de escritura
        if (bytes_written != bytes_read) {
            perror("Fallo crítico: No se pudieron escribir todos los bytes leídos (posible disco lleno)");
            close(fd_in);
            close(fd_out);
            return -1;
        }
    }

    // Si el ciclo termina y bytes_read es menor a 0, hubo un fallo del sistema de archivos.
    if (bytes_read < 0) {
        perror("Fallo crítico durante la lectura del archivo origen");
        close(fd_in);
        close(fd_out);
        return -1;
    }

    // 3. Limpieza y cierre seguro de descriptores
    close(fd_in);
    close(fd_out);
    
    return 0; // 0 significa éxito absoluto en la copia.
}

// Función de copia estándar usando funciones de biblioteca, para comparación y pruebas.
int lib_standard_copy(const char *src, const char *dest) {
    // Abrir archivos en modo binario ("rb" para lectura, "wb" para escritura)
    FILE *in = fopen(src, "rb");
    if (in == NULL) {
        perror("Librería: Error abriendo origen");
        return -1;
    }

    FILE *out = fopen(dest, "wb");
    if (out == NULL) {
        perror("Librería: Error abriendo destino");
        fclose(in);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    // fread gestiona su propio buffering interno en el espacio de usuario
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, in)) > 0) {
        size_t bytes_written = fwrite(buffer, 1, bytes_read, out);
        if (bytes_written != bytes_read) {
            perror("Librería: Error crítico de escritura");
            fclose(in);
            fclose(out);
            return -1;
        }
    }

    // Validación de errores de lectura al salir del ciclo
    if (ferror(in)) {
        perror("Librería: Error leyendo el archivo");
        fclose(in);
        fclose(out);
        return -1;
    }

    fclose(in);
    fclose(out);
    return 0;
}

int compress_to_gzip(const char *src, const char *dest_gz) {
    FILE *in = fopen(src, "rb");
    if (in == NULL) {
        perror("Compresion: Error abriendo archivo origen");
        return -1;
    }

    gzFile out = gzopen(dest_gz, "wb9");
    if (out == NULL) {
        perror("Compresion: Error creando archivo .gz");
        fclose(in);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, in)) > 0) {
        int bytes_written = gzwrite(out, buffer, (unsigned int)bytes_read);
        if (bytes_written == 0) {
            int gz_err = 0;
            const char *gz_msg = gzerror(out, &gz_err);
            fprintf(stderr, "Compresion: Error escribiendo gzip: %s\n", gz_msg);
            fclose(in);
            gzclose(out);
            return -1;
        }
    }

    if (ferror(in)) {
        perror("Compresion: Error leyendo archivo a comprimir");
        fclose(in);
        gzclose(out);
        return -1;
    }

    fclose(in);

    if (gzclose(out) != Z_OK) {
        fprintf(stderr, "Compresion: Error cerrando archivo gzip\n");
        return -1;
    }

    return 0;
}
