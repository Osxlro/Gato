#ifndef UI_H
#define UI_H

// Limpia la pantalla de la consola.
void clearScreen(void);
// Muestra el menú principal en la consola.
void showMainMenu(void);
// Muestra la pantalla de ayuda e instrucciones.
void showHelp(void);
// Pausa la ejecución hasta que el usuario presione Enter.
void pauseEnter(void);
// Dibuja el tablero 3x3 (pasado por argumento) en la consola.
void printBoard(const char board[3][3]);

#endif