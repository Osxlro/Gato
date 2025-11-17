/*
 * main.c
 *
 * Responsabilidades:
 * - Servir como el punto de entrada (función 'main') de la aplicación.
 * - Su única tarea es transferir el control al bucle principal
 * de la aplicación (mainMenuLoop).
 *
 * Notas:
 * - Mantiene el 'main' limpio y simple.
 * - No contiene ninguna lógica de juego o de menú; solo el arranque.
 * - 'main.c' solo debe incluir 'app.h'.
 */

 // El juego.
#include "app.h"

int main(void) {
    return mainMenuLoop();
}