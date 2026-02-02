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
 */

#include "ui.h"    // Prototipos de funciones y definiciones
#include "utils.h" // Para readIn()

#include <stdio.h>

// "Limpia" la pantalla imprimiendo varias líneas nuevas.
void clearScreen(void) {
    // Secuencia ANSI para limpiar pantalla y mover cursor a (0,0)
    // Funciona en Windows 10+, Linux y Mac.
    printf("\033[2J\033[H");
}

// Definición de colores ANSI
#define COL_RESET "\033[0m"
#define COL_RED   "\033[31m"
#define COL_BLUE  "\033[34m"
#define COL_BOLD  "\033[1m"

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
    char tempBuffer[2];
    readIn(tempBuffer, sizeof(tempBuffer));
}

// Helper para imprimir una celda coloreada
static void printCell(char cell) {
    if (cell == 'X') {
        printf(" %s%c%s ", COL_RED, cell, COL_RESET);
    } else if (cell == 'O') {
        printf(" %s%c%s ", COL_BLUE, cell, COL_RESET);
    } else {
        printf("   ");
    }
}

// Dibuja el estado actual del tablero en la consola.
void printBoard(const char board[3][3]) {
    puts("");
    puts("  Col:  1   2   3");
    puts("Fil");
    printf(" 1     ");
    printCell(board[0][0]); printf("|"); printCell(board[0][1]); printf("|"); printCell(board[0][2]);
    printf("\n");
    
    puts("       ---+---+---");
    printf(" 2     ");
    printCell(board[1][0]); printf("|"); printCell(board[1][1]); printf("|"); printCell(board[1][2]);
    printf("\n");

    puts("       ---+---+---");
    printf(" 3     ");
    printCell(board[2][0]); printf("|"); printCell(board[2][1]); printf("|"); printCell(board[2][2]);
    printf("\n");
}