/*
 * utils.c
 *
 * Responsabilidades:
 * - Implementar la función de utilidad 'getSafeLine'.
 *  
 */

 // Librerias del juego
#include "utils.h"

// Librerias del lenguaje
#include <stdio.h>
#include <string.h> // Para strchr
#include <ctype.h>  // Para isalnum()

int readIn(char *buffer, int size) {
    // 1. Leer la línea de forma segura
    if (!fgets(buffer, size, stdin)) {
        // Si fgets falla (devuelve NULL), es un error o EOF
        buffer[0] = '\0'; // Asegurar buffer vacío
        return 0; // Indicar fallo (EOF)
    }

    // 2. Buscar el salto de línea ('\n')
    char *newline = strchr(buffer, '\n');

    if (newline) {
        // Caso A: Se encontró '\n'.
        // La línea completa (incluyendo '\n') cupo en el búfer.
        // Simplemente reemplazamos '\n' con '\0' para limpiar el string.
        *newline = '\0';
    } else {
        // Caso B: No se encontró '\n'.
        // Esto significa que la línea era MÁS LARGA que 'size'.
        // El búfer está lleno, pero stdin TODAVÍA tiene caracteres.
        // Debemos limpiar el resto del búfer de entrada.
        int c;
        // Seguir leyendo y descartando...
        while ((c = getchar()) != '\n' && c != EOF) {
        }
    }

    return 1; // Indicar lectura exitosa
}

void sanitizeString(char *str) {
    if (!str) return;

    char *write_ptr = str; // Puntero de escritura
    char *read_ptr = str;  // Puntero de lectura

    while (*read_ptr) {
        // isalnum() significa "es Alfanumérico" (letra o número)
        // También permitiremos espacios.
        if (isalnum((unsigned char)*read_ptr) || *read_ptr == ' ') {
            // Si es un caracter válido, lo copiamos
            *write_ptr = *read_ptr;
            write_ptr++; // Avanzamos el puntero de escritura
        }
        // Si no es válido (ej. ',', '!', '/'),
        // simplemente no lo copiamos y avanzamos el
        // puntero de lectura.
        read_ptr++;
    }
    // Al final, colocamos el terminador nulo
    *write_ptr = '\0';
}