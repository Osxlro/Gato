#ifndef GAME_H
#define GAME_H

// --- Lógica "Core" del Tablero ---
// (Estas se mantienen públicas porque la IA las necesita)

// Inicializa el tablero.
void initBoard(char board[3][3]);
// Valida rango (1-3).
int  isValidCell(int r, int c);
// Valida si está vacío.
int  isCellEmpty(const char board[3][3], int r, int c);
// Aplica un movimiento.
int  applyMove(char board[3][3], int r, int c, char sym);
// Verifica victoria.
int  checkWin(const char board[3][3], char sym);
// Verifica empate.
int  boardFull(const char board[3][3]);

// --- Flujos de Juego (Entrada de la Aplicación) ---
// (Solo exponemos los "puntos de entrada" al juego)

void playPVP(void);
void playPVC(void);
void playOnline(void);

#endif