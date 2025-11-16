// Librerias del juego
#include "game.h"
#include "ui.h"
#include "io.h"

// Librerias por el Lenguaje
#include <stdio.h>
#include <string.h>

int isValidCell(int r, int c) {
    return r >= 1 && r <= 3 && c >= 1 && c <= 3;
}

void initBoard(char board[3][3]) {
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            board[r][c] = ' ';
}

int isCellEmpty(const char board[3][3], int r, int c) {
    return board[r-1][c-1] == ' ';
}

int applyMove(char board[3][3], int r, int c, char sym) {
    if (!isValidCell(r, c)) return 0;
    if (!isCellEmpty(board, r, c)) return 0;
    board[r-1][c-1] = sym;
    return 1;
}

int checkWin(const char board[3][3], char sym) {
    // filas y columnas
    for (int i = 0; i < 3; ++i) {
        if (board[i][0]==sym && board[i][1]==sym && board[i][2]==sym) return 1;
        if (board[0][i]==sym && board[1][i]==sym && board[2][i]==sym) return 1;
    }
    // diagonales
    if (board[0][0]==sym && board[1][1]==sym && board[2][2]==sym) return 1;
    if (board[0][2]==sym && board[1][1]==sym && board[2][0]==sym) return 1;
    return 0;
}

int boardFull(const char board[3][3]) {
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (board[r][c] == ' ') return 0;
    return 1;
}

int readMove(int *r, int *c) {
    // lee dos enteros; simple, robusteceremos con limpieza de buffer
    int read = scanf("%d %d", r, c);
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {} // limpia hasta fin de línea
    return (read == 2);
}

int scoreOf(int wins, int draws, int losses) {
    (void)losses;
    return 3*wins + 1*draws;
}

/* Stubs: implementaremos la lógica completa en los siguientes pasos */
void playPVP(void) {
    puts("[JvJ] (pendiente de implementación)");
}

void playPVC(void) {
    puts("[JvPC] (pendiente de implementación)");
}