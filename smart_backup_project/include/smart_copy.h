// Se define que se recibe y que se entrega en la función sys_smart_copy, así como el tamaño del buffer que se usará para la copia. 

#ifndef SMART_COPY_H
#define SMART_COPY_H

// Definir el tamaño del buffer aquí para cambiarlo fácilmente durante las pruebas.
#define BUFFER_SIZE 4096 

// Firma de la función: Retorna 0 en éxito, -1 en error.
int sys_smart_copy(const char *src, const char *dest);
// Firma de la función para la copia estándar usando funciones de biblioteca.
int lib_standard_copy(const char *src, const char *dest);

#endif