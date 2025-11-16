#ifndef GAME_H
#define GAME_H

int  isValidCell(int r, int c);
void initBoard(char board[3][3]);
int  isCellEmpty(const char board[3][3], int r, int c);
int  applyMove(char board[3][3], int r, int c, char sym);
int  checkWin(const char board[3][3], char sym);
int  boardFull(const char board[3][3]);
int  readMove(int *r, int *c);
int  scoreOf(int wins, int draws, int losses);

// flujos de juego (se implementarán gradualmente)
void playPVP(void);
void playPVC(void);

// Auxiliar
void askPlayerNames(char p1[], char p2[], int maxLen);

#endif