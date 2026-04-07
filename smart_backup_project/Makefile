# Makefile for smart_backup_project

# Variables del compilador
CC = gcc
# La bandera -Iinclude le dice a GCC que busque los archivos .h en la carpeta "include"
CFLAGS = -Wall -Wextra -O2 -Iinclude 
LDLIBS = -lrt -lz
TARGET = smart_backup
SRC = src/main.c src/backup_engine.c
OBJ = $(SRC:.c=.o)

# Regla principal
all: $(TARGET)

# Cómo enlazar los objetos para crear el ejecutable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDLIBS)

# Cómo compilar cada archivo .c que está dentro de src/
src/%.o: src/%.c include/smart_copy.h
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para limpiar los archivos generados
clean:
	rm -f src/*.o $(TARGET) tests/*_copy.bin tests/*.bin tests/*.gz