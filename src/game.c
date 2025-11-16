// Librerias del juego
#include "game.h"
#include "ui.h"
#include "io.h"
#include "ai.h"

// Librerias por el Lenguaje
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef NAME_MAX
#define NAME_MAX 32
#endif


static void readLine(char *buf, int max) {
    if (!fgets(buf, max, stdin)) { buf[0] = '\0'; return; }
    size_t len = strlen(buf);
    if (len && buf[len-1] == '\n') buf[len-1] = '\0';
}

void askPlayerNames(char p1[], char p2[], int maxLen) {
    printf("Nombre Jugador 1: ");
    readLine(p1, maxLen);
    if (p1[0] == '\0') strncpy(p1, "Jugador1", maxLen);

    printf("Nombre Jugador 2: ");
    readLine(p2, maxLen);
    if (p2[0] == '\0') strncpy(p2, "Jugador2", maxLen);
}

void askHumanName(char p1[], int maxLen) {
    printf("Tu nombre: ");
    readLine(p1, maxLen);
    if (p1[0] == '\0') strncpy(p1, "Humano", maxLen);
}

//


static int seeded = 0;
static void ensureSeed(void) {
    if (!seeded) { seeded = 1; srand((unsigned)time(NULL)); }
}
int randomStarts(void) { ensureSeed(); return rand() & 1; } /* 0 o 1 */



/* ---------- JvJ con inicio aleatorio ---------- */
void playPVP(void) {
    char name1[NAME_MAX], name2[NAME_MAX];
    askPlayerNames(name1, name2, NAME_MAX);

    char board[3][3];
    initBoard(board);

    /* Quien inicia lleva 'X' */
    int starter = randomStarts();      /* 0 -> name1 empieza, 1 -> name2 empieza */
    int current = starter;             /* 0 o 1 */
    int r, c;

    int wins[2]  = {0,0};
    int draws[2] = {0,0};
    int losses[2]= {0,0};

    for (;;) {
        clearScreen();
        printf("Modo: Jugador vs Jugador\n");
        printf("Inicia: %s (X)\n", starter==0 ? name1 : name2);
        printf("Turno:  %s (%c)\n",
               current==0 ? name1 : name2,
               (current==starter) ? 'X' : 'O');
        printBoard(board);

        printf("Ingresa fila y columna (1..3 1..3): ");
        if (!readMove(&r, &c)) { puts("Entrada invalida."); pauseEnter(); continue; }
        if (!isValidCell(r, c)) { puts("Fuera de rango (1..3)."); pauseEnter(); continue; }
        if (!isCellEmpty(board, r, c)) { puts("Casilla ocupada."); pauseEnter(); continue; }

        char sym = (current==starter) ? 'X' : 'O';
        applyMove(board, r, c, sym);

        if (checkWin(board, sym)) {
            clearScreen(); printBoard(board);
            printf("\nGana %s (%c)\n", current==0 ? name1 : name2, sym);
            wins[current]++; losses[1-current]++;
            break;
        }
        if (boardFull(board)) {
            clearScreen(); printBoard(board);
            puts("\nEmpate!");
            draws[0]++; draws[1]++;
            break;
        }
        current = 1 - current;
    }

    int score0 = scoreOf(wins[0], draws[0], losses[0]);
    int score1 = scoreOf(wins[1], draws[1], losses[1]);

    if (!upsertResult(name1, wins[0], draws[0], losses[0], score0))
        puts("\n[ADVERTENCIA] No se pudo actualizar ranking para Jugador 1.");
    if (!upsertResult(name2, wins[1], draws[1], losses[1], score1))
        puts("[ADVERTENCIA] No se pudo actualizar ranking para Jugador 2.");

    printf("\nJugar otra partida JvJ? (s/n): ");
    int ch = getchar(); int dump; while ((dump=getchar())!='\n' && dump!=EOF){}
    if (ch=='s'||ch=='S') playPVP();
}



/* ---------- JvPC con inicio aleatorio ---------- */
extern void pcMove(char board[3][3], char pcSym, char humanSym);

void playPVC(void) {
    char human[NAME_MAX];
    const char pcName[] = "PC";
    askHumanName(human, NAME_MAX);

    char board[3][3];
    initBoard(board);

    int starter = randomStarts(); /* 0 -> humano inicia (X), 1 -> PC inicia (X) */
    int current = starter;

    int winsH=0, drawsH=0, lossesH=0;
    int winsPC=0, drawsPC=0, lossesPC=0;

    int r,c;
    for (;;) {
        clearScreen();
        printf("Modo: Jugador vs PC\n");
        printf("Inicia: %s (X)\n", starter==0 ? human : pcName);
        printf("Turno:  %s (%c)\n", current==0 ? human : pcName,
               (current==starter) ? 'X' : 'O');
        printBoard(board);

        if (current == 0) { /* Humano */
            printf("Ingresa fila y columna (1..3 1..3): ");
            if (!readMove(&r,&c)) { puts("Entrada invalida."); pauseEnter(); continue; }
            if (!isValidCell(r,c)) { puts("Fuera de rango (1..3)."); pauseEnter(); continue; }
            if (!isCellEmpty(board,r,c)) { puts("Casilla ocupada."); pauseEnter(); continue; }
            char sym = (starter==0) ? 'X' : 'O';
            applyMove(board, r, c, sym);
            if (checkWin(board, sym)) {
                clearScreen(); printBoard(board);
                printf("\nGana %s (%c)\n", human, sym);
                winsH++; lossesPC++; break;
            }
        } else { /* PC */
            char sym = (starter==1) ? 'X' : 'O';
            pcMove(board, sym, (sym=='X') ? 'O' : 'X');
            if (checkWin(board, sym)) {
                clearScreen(); printBoard(board);
                printf("\nGana %s (%c)\n", pcName, sym);
                winsPC++; lossesH++; break;
            }
        }

        if (boardFull(board)) {
            clearScreen(); printBoard(board);
            puts("\nEmpate!");
            drawsH++; drawsPC++; break;
        }
        current = 1 - current;
    }

    int scoreH  = scoreOf(winsH, drawsH, lossesH);
    int scorePC = scoreOf(winsPC, drawsPC, lossesPC);

    if (!upsertResult(human, winsH, drawsH, lossesH, scoreH))
        puts("\n[ADVERTENCIA] No se pudo actualizar ranking para Humano.");
    if (!upsertResult(pcName, winsPC, drawsPC, lossesPC, scorePC))
        puts("[ADVERTENCIA] No se pudo actualizar ranking para PC.");

    printf("\nJugar otra partida JvPC? (s/n): ");
    int ch = getchar(); int dump; while ((dump=getchar())!='\n' && dump!=EOF){}
    if (ch=='s'||ch=='S') playPVC();
}




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
