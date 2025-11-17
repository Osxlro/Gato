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


/* Funciones de IA */

// Busca una jugada ganadora inmediata para 'simbolo (sym)' y la aplica si la encuentra.
// Devuelve 1 si encontró y aplicó la jugada, 0 si no.
static int tryWin(char board[3][3], char sym) {
    // Coloca sym en alguna celda vacía que produzca victoria inmediata.
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            if (board[r][c] == ' ') {
                board[r][c] = sym; // Probar jugada
                int win = checkWin(board, sym);
                if (win) return 1;     // Victoria, Dejamos la jugada puesta.
                board[r][c] = ' ';     // Revertimos la jugada si no era ganadora.
            }
        }
    }
    return 0; // No se encontró jugada ganadora.
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

/* IA Principal */

// Función principal de la IA: decide y aplica el mejor movimiento.
void pcMove(char board[3][3], char pcSym, char humanSym) {
    
    // Estrategias:
    // 1) Ganar si es posible
    if (tryWin(board, pcSym)) return;

    // 2) Bloquear victoria del Jugador
    // Nota: Este doble bucle es O(N^2) para un tablero de N x N (aquí N=3), lo cual es aceptable para el juego,
    // pero podría ser ineficiente en tableros más grandes.
    //Coloca temporalmente la jugada del Jugador; si gana, la bloquea poniendo pcSym
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            if (board[r][c] == ' ') {
                board[r][c] = humanSym; // Simular jugada humana
                int humanWin = checkWin(board, humanSym);
                board[r][c] = ' '; // Revertir simulación
                if (humanWin) {
                    board[r][c] = pcSym; // ¡Bloquear aquí!
                    return;
                }
            }
        }
    }

    // 3) Tomar centro
    if (takeCenter(board, pcSym)) return;

    // 4) Tomar esquina libre (coordenadas 0-index)
    static const int corners[4][2] = { {0,0},{0,2},{2,0},{2,2} };
    if (takeFirst(board, pcSym, corners, 4)) return;

    // 5) Tomar lateral libre (coordenadas 0-index)
    static const int sides[4][2]   = { {0,1},{1,0},{1,2},{2,1} };
    if (takeFirst(board, pcSym, sides, 4)) return;

    // 6) Tomar la primera libre (no debería ser necesario si la lógica anterior cubre todos los casos).
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (board[r][c] == ' ') { board[r][c] = pcSym; return; }
}