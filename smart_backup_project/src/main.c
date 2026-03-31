// Este archivo corresponde a la interfaz de usuario. Recibirá los comandos por consola, ejecutará las copias y medirá el rendimiento.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "smart_copy.h"

// Declarar la función de la librería 
int lib_standard_copy(const char *src, const char *dest);

// Función auxiliar para calcular la diferencia de tiempo en segundos
double calculate_time(struct timespec start, struct timespec end) {
    // Convertir los segundos y los nanosegundos a un solo valor decimal
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(int argc, char *argv[]) {
    // 1. Validación técnica: Asegurar que el usuario entregue las rutas
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <origen> <destino_sys> <destino_lib>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *src = argv[1];
    const char *dest_sys = argv[2];
    const char *dest_lib = argv[3];

    struct timespec start, end;
    double time_sys, time_lib;

    printf("Iniciando pruebas de rendimiento con BUFFER_SIZE = %d bytes\n", BUFFER_SIZE);
    printf("----------------------------------------------------\n");

    // --- PRUEBA 1: Syscall Simulada ---
    clock_gettime(CLOCK_MONOTONIC, &start); // Inicializar el cronómetro
    
    if (sys_smart_copy(src, dest_sys) == 0) {
        clock_gettime(CLOCK_MONOTONIC, &end); // Detener el cronómetro
        time_sys = calculate_time(start, end);
        printf("[Syscall] Copia exitosa. Tiempo: %.6f segundos\n", time_sys);
    } else {
        printf("[Syscall] Fallo en la copia.\n");
    }

    // --- PRUEBA 2: Librería Estándar (stdio.h) ---
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    if (lib_standard_copy(src, dest_lib) == 0) {
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_lib = calculate_time(start, end);
        printf("[Librería] Copia exitosa. Tiempo: %.6f segundos\n", time_lib);
    } else {
        printf("[Librería] Fallo en la copia.\n");
    }

    return EXIT_SUCCESS;
}
