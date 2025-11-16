#include "ai.h"
#include "game.h"

/* Estrategia placeholder: más adelante añadimos
   ganar->bloquear->centro->esquina->lateral */
void pcMove(char board[3][3], char pcSym, char humanSym) {
    (void)humanSym;
    // Búsqueda simple de primera casilla libre:
    for (int r = 1; r <= 3; ++r) {
        for (int c = 1; c <= 3; ++c) {
            if (applyMove(board, r, c, pcSym)) return;
        }
    }
}