/*
 * ui.c
 *
 * Responsabilidades:
 * - Proveer funciones para interactuar con la consola (Interfaz de Usuario).
 * - "Limpiar" la terminal (clearScreen).
 * - Mostrar menús estáticos (showMainMenu, showHelp).
 * - Pausar la ejecución del programa (pauseEnter).
 * - Renderizar el estado actual del tablero de juego (printBoard).
 *
 * Notas:
 * - 'clearScreen' es una implementación portable (imprime líneas nuevas)
 * para evitar 'system("clear")' o 'system("cls")'.
 *
 * Posibles bugs:
 * - 'pauseEnter' usa 'getchar' y asume que el búfer está limpio,
 * excepto por un '\n' previo. Si el búfer contiene más caracteres,
 * podría no pausar o pausar varias veces.
 */

#include "ui.h"
#include <stdio.h>

// "Limpia" la pantalla imprimiendo varias líneas nuevas.
void clearScreen(void) {
    for (int i = 0; i < 30; ++i) puts("");
}

// Muestra el menú principal del juego.
void showMainMenu(void) {
    puts("+--------------------------------------+");
    puts("|              (Gato)                  |");
    puts("+--------------------------------------+");
    puts("| 1) Jugar                             |");
    puts("| 2) Tabla de posiciones (ranking)     |");
    puts("| 3) Ayuda / Instrucciones             |");
    puts("| 4) Salir                             |");
    puts("+--------------------------------------+");
    printf("Seleccione una opcion: ");
}

// Muestra las instrucciones y reglas del juego.
void showHelp(void) {
    puts("\n[AYUDA]");
    puts("- Objetivo: Alinear 3 simbolos iguales (X u O).\n");
    puts("- Entrada:  Coordenadas fila columna ('1 3'..'1 4'..).\n");
    puts("- Puntaje sugerido:"); 
    puts("  G=3, E=1, P=0.\n");
    puts("Se guarda el Score del usuario en el Ranking.\n");
}

// Pausa la ejecución hasta que el usuario presione Enter.
void pauseEnter(void) {
    puts("\nPresiona Enter para continuar...");
    int c;
    // Limpiar buffer
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// Dibuja el estado actual del tablero en la consola.
void printBoard(const char board[3][3]) {
    puts("");
    puts("  Col:  1   2   3");
    puts("Fil");
    printf(" 1      %c | %c | %c \n",
        board[0][0] ? board[0][0] : ' ',
        board[0][1] ? board[0][1] : ' ',
        board[0][2] ? board[0][2] : ' ');
    puts("       ---+---+---");
    printf(" 2      %c | %c | %c \n",
        board[1][0] ? board[1][0] : ' ',
        board[1][1] ? board[1][1] : ' ',
        board[1][2] ? board[1][2] : ' ');
    puts("       ---+---+---");
    printf(" 3      %c | %c | %c \n",
        board[2][0] ? board[2][0] : ' ',
        board[2][1] ? board[2][1] : ' ',
        board[2][2] ? board[2][2] : ' ');
}