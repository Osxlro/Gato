#include "ai.h"
#include "game.h"


/* Helpers internos */
static int tryWin(char board[3][3], char sym) {
    // Coloca sym en alguna celda vacía que produzca victoria inmediata.
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            if (board[r][c] == ' ') {
                board[r][c] = sym;
                int win = checkWin(board, sym);
                if (win) return 1;     // dejamos la jugada puesta
                board[r][c] = ' ';     // revertimos
            }
        }
    }
    return 0;
}

static int takeCenter(char board[3][3], char sym) {
    if (board[1][1] == ' ') {
        board[1][1] = sym;
        return 1;
    }
    return 0;
}

static int takeFirst(char board[3][3], char sym, const int coords[][2], int n) {
    for (int i = 0; i < n; ++i) {
        int r = coords[i][0], c = coords[i][1];
        if (board[r][c] == ' ') {
            board[r][c] = sym;
            return 1;
        }
    }
    return 0;
}

void pcMove(char board[3][3], char pcSym, char humanSym) {
    // 1) Ganar si es posible
    if (tryWin(board, pcSym)) return;

    // 2) Bloquear victoria del humano
    //    Colocamos temporalmente la jugada del humano; si gana, la bloqueamos poniendo pcSym
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            if (board[r][c] == ' ') {
                board[r][c] = humanSym;
                int humanWin = checkWin(board, humanSym);
                board[r][c] = ' ';
                if (humanWin) {
                    board[r][c] = pcSym; // bloquear aquí
                    return;
                }
            }
        }
    }

    // 3) Tomar centro
    if (takeCenter(board, pcSym)) return;

    // 4) Tomar esquina libre
    static const int corners[4][2] = { {0,0},{0,2},{2,0},{2,2} };
    if (takeFirst(board, pcSym, corners, 4)) return;

    // 5) Tomar lateral libre
    static const int sides[4][2]   = { {0,1},{1,0},{1,2},{2,1} };
    if (takeFirst(board, pcSym, sides, 4)) return;

    // 6) Fallback: primera libre (no debería ser necesario, pero por seguridad)
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (board[r][c] == ' ') { board[r][c] = pcSym; return; }
}