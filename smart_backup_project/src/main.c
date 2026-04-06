// Este archivo corresponde a la interfaz de usuario. Recibirá los comandos por consola, ejecutará las copias y medirá el rendimiento.

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "smart_copy.h"

// Función auxiliar para calcular la diferencia de tiempo en segundos
double calculate_time(struct timespec start, struct timespec end) {
    // Convertir los segundos y los nanosegundos a un solo valor decimal
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

static char *build_gzip_path(const char *path) {
    size_t len = strlen(path);
    char *out = malloc(len + 4);
    if (out == NULL) {
        return NULL;
    }
    snprintf(out, len + 4, "%s.gz", path);
    return out;
}

static char *build_temp_gzip_path(const char *path) {
    size_t len = strlen(path);
    const char *suffix = ".backup_tmp.gz";
    size_t suffix_len = strlen(suffix);
    char *out = malloc(len + suffix_len + 1);
    if (out == NULL) {
        return NULL;
    }
    snprintf(out, len + suffix_len + 1, "%s%s", path, suffix);
    return out;
}

int main(int argc, char *argv[]) {
    int compress_after = 0;
    int compress_first = 0;
    const char *src;
    const char *dest_sys;
    const char *dest_lib;
    const char *copy_source;
    char *temp_src_gz = NULL;

    // Uso clásico: ./smart_backup <origen> <destino_sys> <destino_lib>
    if (argc == 4) {
        src = argv[1];
        dest_sys = argv[2];
        dest_lib = argv[3];
    } else if (argc == 5 && strcmp(argv[1], "--compress-after") == 0) {
        // Uso extendido: ./smart_backup --compress-after <origen> <destino_sys> <destino_lib>
        compress_after = 1;
        src = argv[2];
        dest_sys = argv[3];
        dest_lib = argv[4];
    } else if (argc == 5 && strcmp(argv[1], "--compress-first") == 0) {
        // Uso extendido: ./smart_backup --compress-first <origen> <destino_sys> <destino_lib>
        compress_first = 1;
        src = argv[2];
        dest_sys = argv[3];
        dest_lib = argv[4];
    } else {
        fprintf(stderr, "Uso: %s [--compress-after|--compress-first] <origen> <destino_sys> <destino_lib>\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct timespec start, end, total_start, total_end;
    double time_sys = 0.0;
    double time_lib = 0.0;
    double time_comp_src = 0.0;
    double time_comp_sys = 0.0;
    double time_comp_lib = 0.0;
    double time_total = 0.0;
    int sys_ok = 0;
    int lib_ok = 0;

    copy_source = src;

    printf("Iniciando pruebas de rendimiento con BUFFER_SIZE = %d bytes\n", BUFFER_SIZE);
    if (compress_after) {
        printf("Modo adicional: compresion post-backup activada\n");
    } else if (compress_first) {
        printf("Modo adicional: compresion previa al backup activada\n");
    }
    printf("----------------------------------------------------\n");

    clock_gettime(CLOCK_MONOTONIC, &total_start);

    if (compress_first) {
        temp_src_gz = build_temp_gzip_path(src);
        if (temp_src_gz == NULL) {
            fprintf(stderr, "[Compresion Inicial] Sin memoria para construir archivo temporal\n");
            return EXIT_FAILURE;
        }

        clock_gettime(CLOCK_MONOTONIC, &start);
        if (compress_to_gzip(src, temp_src_gz) != 0) {
            fprintf(stderr, "[Compresion Inicial] No se pudo comprimir el origen antes del backup\n");
            free(temp_src_gz);
            return EXIT_FAILURE;
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_comp_src = calculate_time(start, end);
        copy_source = temp_src_gz;
        printf("[Compresion Inicial] Archivo temporal generado: %s (%.6f segundos)\n", temp_src_gz, time_comp_src);
        printf("----------------------------------------------------\n");
    }

    // --- PRUEBA 1: Syscall Simulada ---
    clock_gettime(CLOCK_MONOTONIC, &start); // Inicializar el cronómetro
    
    if (sys_smart_copy(copy_source, dest_sys) == 0) {
        clock_gettime(CLOCK_MONOTONIC, &end); // Detener el cronómetro
        time_sys = calculate_time(start, end);
        printf("[Syscall] Copia exitosa. Tiempo: %.6f segundos\n", time_sys);
        sys_ok = 1;
    } else {
        printf("[Syscall] Fallo en la copia.\n");
    }

    // --- PRUEBA 2: Librería Estándar (stdio.h) ---
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    if (lib_standard_copy(copy_source, dest_lib) == 0) {
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_lib = calculate_time(start, end);
        printf("[Librería] Copia exitosa. Tiempo: %.6f segundos\n", time_lib);
        lib_ok = 1;
    } else {
        printf("[Librería] Fallo en la copia.\n");
    }

    if (compress_after) {
        printf("----------------------------------------------------\n");
        printf("Iniciando compresion de respaldos generados...\n");

        if (sys_ok) {
            char *sys_gz = build_gzip_path(dest_sys);
            if (sys_gz == NULL) {
                fprintf(stderr, "[Compresion Syscall] Sin memoria para construir ruta .gz\n");
            } else {
                clock_gettime(CLOCK_MONOTONIC, &start);
                if (compress_to_gzip(dest_sys, sys_gz) == 0) {
                    clock_gettime(CLOCK_MONOTONIC, &end);
                    time_comp_sys = calculate_time(start, end);
                    printf("[Compresion Syscall] Archivo comprimido: %s (%.6f segundos)\n", sys_gz, time_comp_sys);
                } else {
                    printf("[Compresion Syscall] Fallo al comprimir %s\n", dest_sys);
                }
            }
            free(sys_gz);
        }

        if (lib_ok) {
            char *lib_gz = build_gzip_path(dest_lib);
            if (lib_gz == NULL) {
                fprintf(stderr, "[Compresion Libreria] Sin memoria para construir ruta .gz\n");
            } else {
                clock_gettime(CLOCK_MONOTONIC, &start);
                if (compress_to_gzip(dest_lib, lib_gz) == 0) {
                    clock_gettime(CLOCK_MONOTONIC, &end);
                    time_comp_lib = calculate_time(start, end);
                    printf("[Compresion Libreria] Archivo comprimido: %s (%.6f segundos)\n", lib_gz, time_comp_lib);
                } else {
                    printf("[Compresion Libreria] Fallo al comprimir %s\n", dest_lib);
                }
            }
            free(lib_gz);
        }
    }

    if (compress_first && temp_src_gz != NULL) {
        if (remove(temp_src_gz) != 0) {
            fprintf(stderr, "[Compresion Inicial] Aviso: no se pudo borrar temporal %s\n", temp_src_gz);
        }
        free(temp_src_gz);
    }

    clock_gettime(CLOCK_MONOTONIC, &total_end);
    time_total = calculate_time(total_start, total_end);

    printf("----------------------------------------------------\n");
    if (compress_after) {
        printf("[Resumen] Tiempo total backup->compresion: %.6f segundos\n", time_total);
    } else if (compress_first) {
        printf("[Resumen] Tiempo total compresion->backup: %.6f segundos\n", time_total);
    } else {
        printf("[Resumen] Tiempo total backup simple: %.6f segundos\n", time_total);
    }

    return EXIT_SUCCESS;
}
