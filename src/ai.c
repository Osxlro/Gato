/*
 * ai.c
 *
 * Responsabilidades:
 * - Lógica de la Inteligencia Artificial (IA) para PVC (Jugador vs Computadora).
 * - Proveer la función principal 'pcMove' que decide el siguiente movimiento.
 * - Contener funciones auxiliares para sus estrategias de la IA (tryWin, takeCenter, takeFirst).
 *
 * Notas:
 * - La IA sigue una estrategia simple:
 * 1. Ganar si es posible (tryWin).
 * 2. Bloquear al humano si puede ganar.
 * 3. Tomar el centro (takeCenter).
 * 4. Tomar una esquina (takeFirst).
 * 5. Tomar un lateral (takeFirst).
 * - Esta estrategia es suficiente para ser "imbatible" (nunca pierde, puede empatar o ganar).
 *
 * Posibles bugs:
 * - Mira la linea 77 (pcMove) para un posible problema de eficiencia en tableros más grandes.
 */

 // Librerias del Juego
#include "ai.h"
#include "game.h" // Necesita checkWin, applyMove, etc.
#include <stdlib.h> // Para rand()


/* Funciones de IA */

// Busca si existe una jugada que otorgue la victoria inmediata a 'sym'.
// Si la encuentra, guarda las coordenadas en outR y outC y devuelve 1.
static int findWinningMove(char board[3][3], char sym, int *outR, int *outC) {
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            if (board[r][c] == ' ') {
                board[r][c] = sym; // Simular
                int win = checkWin(board, sym);
                board[r][c] = ' '; // Revertir
                if (win) {
                    *outR = r;
                    *outC = c;
                    return 1;
                }
            }
        }
    }
}

// Ocupa la casilla central (1,1 en coordenadas 0-index) si está libre.
static int takeCenter(char board[3][3], char sym) {
    if (board[1][1] == ' ') {
        board[1][1] = sym;
        return 1;
    }
    return 0;
}

// Ocupa la primera casilla libre de una lista de coordenadas dadas (esquinas/laterales).
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

// Mezcla un array de coordenadas para dar variedad a la IA
static void shuffleCoords(int coords[][2], int n) {
    for (int i = 0; i < n - 1; i++) {
        int j = i + rand() / (RAND_MAX / (n - i) + 1);
        int t0 = coords[j][0]; int t1 = coords[j][1];
        coords[j][0] = coords[i][0]; coords[j][1] = coords[i][1];
        coords[i][0] = t0; coords[i][1] = t1;
    }
}

/* IA Principal */

// Función principal de la IA: decide y aplica el mejor movimiento.
void pcMove(char board[3][3], char pcSym, char humanSym) {
    
    int r, c;

    // Estrategias:
    // 1) Ganar si es posible
    if (findWinningMove(board, pcSym, &r, &c)) {
        board[r][c] = pcSym;
        return;
    }

    // 2) Bloquear victoria del Jugador
    // Reutilizamos la lógica de búsqueda para ver si el humano ganaría.
    if (findWinningMove(board, humanSym, &r, &c)) {
        board[r][c] = pcSym; // Bloquear
        return;
    }

    // 3) Tomar centro
    if (takeCenter(board, pcSym)) return;

    // 4) Tomar esquina libre (coordenadas 0-index)
    int corners[4][2] = { {0,0},{0,2},{2,0},{2,2} };
    shuffleCoords(corners, 4); // Aleatorizar orden
    if (takeFirst(board, pcSym, corners, 4)) return;

    // 5) Tomar lateral libre (coordenadas 0-index)
    int sides[4][2]   = { {0,1},{1,0},{1,2},{2,1} };
    shuffleCoords(sides, 4); // Aleatorizar orden
    if (takeFirst(board, pcSym, sides, 4)) return;

    // 6) Tomar la primera libre (no debería ser necesario si la lógica anterior cubre todos los casos).
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (board[r][c] == ' ') { board[r][c] = pcSym; return; }
}