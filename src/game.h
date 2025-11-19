#ifndef GAME_H
#define GAME_H

// --- Lógica del Tablero ---

// Verifica si la celda (r, c) está en el rango [1, 3].
int  isValidCell(int r, int c);
// Inicializa el tablero 3x3 con espacios ' '.
void initBoard(char board[3][3]);
// Verifica si la celda (r, c) [1-indexado] está vacía.
int  isCellEmpty(const char board[3][3], int r, int c);
// Aplica el símbolo 'sym' en la celda (r, c) [1-indexado]. Devuelve 1 si éxito.
int  applyMove(char board[3][3], int r, int c, char sym);
// Verifica si el jugador con el símbolo 'sym' ganó (3 en raya).
int  checkWin(const char board[3][3], char sym);
// Verifica si el tablero está lleno (lo que resulta en empate).
int  boardFull(const char board[3][3]);

// --- Entrada y Puntuación ---

// Lee la entrada del usuario (fila, columna) desde la consola.
 /*
 * 1: Éxito (se leyeron dos números).
 * 0: Formato inválido (ej. "a", "1", "1 2 3").
 * -1: El usuario introdujo 'q' para abandonar.
 */
int  readMove(int *r, int *c);
// Calcula el puntaje (Victoria=3, Empate=1, Derrota=0).
int  scoreOf(int wins, int draws, int losses);

// --- Configuración de Jugadores ---

// Pide los nombres para el modo JvJ.
void askPlayerNames(char p1[], char p2[], int maxLen);
// Pide el nombre del jugador para el modo JvPC.
void askHumanName(char p1[], int maxLen);
// Devuelve 0 o 1 aleatoriamente para decidir quién empieza.
int  randomStarts(void);


// --- Flujos de Juego (Loops) ---

// Inicia el bucle de juego Jugador vs Jugador.
void playPVP(void);
// Inicia el bucle de juego Jugador vs PC.
void playPVC(void);
// Inicia el bucle de juego Jugador vs Jugador Online.
void playOnline(void);
#endif