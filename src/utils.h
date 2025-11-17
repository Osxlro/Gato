#ifndef UTILS_H
#define UTILS_H


/*
 * Lee una línea de forma segura desde stdin.
 * - Garantiza que el 'buffer' siempre termine en '\0'.
 * - Previene desbordamientos de búfer.
 * - Elimina el '\n' final.
 * - Si la línea ingresada por el usuario es más larga
 * que 'size', esta función descarta los caracteres
 * extra de stdin, limpiando el búfer de entrada.
 *
 * Devuelve:
 * - 1 si la lectura fue exitosa.
 * - 0 si se encontró EOF (Fin de Archivo).
 */

int readIn (char *buffer, int size);

/*
 * "Sanea" un string (lo filtra en el lugar).
 * Elimina todos los caracteres excepto letras (a-z, A-Z),
 * números (0-9) y espacios.
 * Previene que caracteres como ',' rompan el formato CSV.
 */
void sanitizeString(char *str);

#endif