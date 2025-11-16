#include "ui.h"
#include <stdio.h>

void clearScreen(void) {
    // Opción simple y portable: imprimir varias líneas en lugar de system("clear")
    for (int i = 0; i < 30; ++i) puts("");
}

void showMainMenu(void) {
    puts("+--------------------------------------+");
    puts("|           TIC-TAC-TOE (Gato)         |");
    puts("+--------------------------------------+");
    puts("| 1) Jugar                              |");
    puts("| 2) Tabla de posiciones (ranking)      |");
    puts("| 3) Ayuda / Instrucciones              |");
    puts("| 4) Salir                              |");
    puts("+--------------------------------------+");
    printf("Seleccione una opción: ");
}

void showHelp(void) {
    puts("\n[AYUDA]");
    puts("- Objetivo: alinear 3 simbolos iguales (X u O).");
    puts("- Entrada: coordenadas fila columna (1..3 1..3).");
    puts("- Puntaje sugerido: G=3, E=1, P=0.");
    puts("- Se guarda ranking en archivo CSV al finalizar la partida.");
}

void pauseEnter(void) {
    puts("\nPresiona Enter para continuar...");
    int c;
    // limpiar buffer hasta newline
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void printBoard(const char board[3][3]) {
    puts("\n    Col: 1   2   3");
    puts("Fil       |   |   ");
    for (int r = 0; r < 3; ++r) {
        printf(" %d     %c | %c | %c \n", r+1,
               board[r][0] ? board[r][0] : ' ',
               board[r][1] ? board[r][1] : ' ',
               board[r][2] ? board[r][2] : ' ');
        if (r < 2) puts("      ---+---+---");
    }
}