// Librerias del juego
#include "game.h"
#include "ui.h"
#include "io.h"

// Librerias por el Lenguaje
#include <stdio.h>
#include <string.h>

#ifndef NAME_MAX
#define NAME_MAX 32
#endif


static void readLine(char *buf, int max) {
    if (!fgets(buf, max, stdin)) {
        buf[0] = '\0';
        return;
    }
    
    size_t len = strlen(buf);
    if (len && buf[len-1] == '\n') buf[len-1] = '\0';
}

void askPlayerNames(char p1[], char p2[], int maxLen) {
    printf("Nombre Jugador 1 (X): ");
    readLine(p1, maxLen);
    if (p1[0] == '\0') strncpy(p1, "Jugador1", maxLen);

    
    printf("Nombre Jugador 2 (O): ");
    readLine(p2, maxLen);
    if (p2[0] == '\0') strncpy(p2, "Jugador2", maxLen);
}

static void askHumanName(char p1[], int maxLen) {
    printf("Tu nombre (X): ");
    readLine(p1, maxLen);
    if (p1[0] == '\0') strncpy(p1, "Humano", maxLen);
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

void playPVP(void) {
    char name1[NAME_MAX], name2[NAME_MAX];
    askPlayerNames(name1, name2, NAME_MAX);

    char board[3][3];
    initBoard(board);

    int wins1 = 0, draws1 = 0, losses1 = 0;
    int wins2 = 0, draws2 = 0, losses2 = 0;

    int current = 0;            // 0 -> P1 (X), 1 -> P2 (O)
    int r, c;
    char sym;

    for (;;) {
        clearScreen();
        printf("Modo: Jugador vs Jugador\n");
        printf("Turno: %s (%c)\n", current == 0 ? name1 : name2, current == 0 ? 'X' : 'O');
        printBoard(board);

        printf("Ingresa fila y columna (1..3 1..3): ");
        if (!readMove(&r, &c)) {
            puts("Entrada inválida. Intenta de nuevo.");
            pauseEnter();
            continue;
        }
        if (!isValidCell(r, c)) {
            puts("Coordenadas fuera de rango (1..3).");
            pauseEnter();
            continue;
        }
        if (!isCellEmpty(board, r, c)) {
            puts("La casilla está ocupada.");
            pauseEnter();
            continue;
        }

        sym = (current == 0) ? 'X' : 'O';
        applyMove(board, r, c, sym);

        // ¿ganó alguien?
        if (checkWin(board, sym)) {
            clearScreen();
            printBoard(board);
            if (current == 0) {
                printf("\n¡Gana %s (X)!\n", name1);
                wins1++; losses2++;
            } else {
                printf("\n¡Gana %s (O)!\n", name2);
                wins2++; losses1++;
            }
            break;
        }

        // ¿empate?
        if (boardFull(board)) {
            clearScreen();
            printBoard(board);
            puts("\n¡Empate!");
            draws1++; draws2++;
            break;
        }

        // Cambiar turno
        current = 1 - current;
    }

    // Calcular puntajes y guardar resultados individuales
    int score1 = scoreOf(wins1, draws1, losses1);
    int score2 = scoreOf(wins2, draws2, losses2);

    // Guardar resultados (cada jugador forma una fila en el CSV)
    if (!saveResult(name1, wins1, draws1, losses1, score1)) {
        puts("\n[ADVERTENCIA] No se pudo guardar el resultado de P1 en ranking.csv");
    }
    if (!saveResult(name2, wins2, draws2, losses2, score2)) {
        puts("[ADVERTENCIA] No se pudo guardar el resultado de P2 en ranking.csv");
    }

    // ¿Jugar de nuevo?
    printf("\n¿Jugar otra partida JvJ? (s/n): ");
    int ch = getchar();
    int dump; while ((dump = getchar()) != '\n' && dump != EOF) {}
    if (ch == 's' || ch == 'S') {
        playPVP(); // recursivo para simplificar; también puedes usar un bucle externo
    }
}



void playPVC(void) {
    char human[NAME_MAX];
    const char pcName[] = "PC";
    askHumanName(human, NAME_MAX);

    char board[3][3];
    initBoard(board);

    // Contadores por partida
    int winsH = 0, drawsH = 0, lossesH = 0;
    int winsPC = 0, drawsPC = 0, lossesPC = 0;

    int r, c;
    // Humano = 'X' (empieza), PC = 'O'
    for (;;) {
        // --- Turno Humano ---
        for (;;) {
            clearScreen();
            printf("Modo: Jugador vs PC\n");
            printf("Turno: %s (X)\n", human);
            printBoard(board);

            printf("Ingresa fila y columna (1..3 1..3): ");
            if (!readMove(&r, &c)) {
                puts("Entrada inválida. Intenta de nuevo.");
                pauseEnter();
                continue;
            }
            if (!isValidCell(r, c)) {
                puts("Coordenadas fuera de rango (1..3).");
                pauseEnter();
                continue;
            }
            if (!isCellEmpty(board, r, c)) {
                puts("La casilla está ocupada.");
                pauseEnter();
                continue;
            }
            applyMove(board, r, c, 'X');
            break;
        }

        // ¿Gana Humano?
        if (checkWin(board, 'X')) {
            clearScreen();
            printBoard(board);
            printf("\n¡Gana %s (X)!\n", human);
            winsH++; lossesPC++;
            break;
        }
        // ¿Empate?
        if (boardFull(board)) {
            clearScreen();
            printBoard(board);
            puts("\n¡Empate!");
            drawsH++; drawsPC++;
            break;
        }

        // --- Turno PC ---
        // IA elegirá una casilla y colocará 'O'
        extern void pcMove(char board[3][3], char pcSym, char humanSym);
        pcMove(board, 'O', 'X');

        // ¿Gana PC?
        if (checkWin(board, 'O')) {
            clearScreen();
            printBoard(board);
            printf("\n¡Gana %s (O)!\n", pcName);
            winsPC++; lossesH++;
            break;
        }
        // ¿Empate?
        if (boardFull(board)) {
            clearScreen();
            printBoard(board);
            puts("\n¡Empate!");
            drawsH++; drawsPC++;
            break;
        }
    }

    // Calcular puntajes y guardar
    int scoreH  = scoreOf(winsH, drawsH, lossesH);
    int scorePC = scoreOf(winsPC, drawsPC, lossesPC);

    if (!saveResult(human, winsH, drawsH, lossesH, scoreH)) {
        puts("\n[ADVERTENCIA] No se pudo guardar el resultado del humano en ranking.csv");
    }
    if (!saveResult(pcName, winsPC, drawsPC, lossesPC, scorePC)) {
        puts("[ADVERTENCIA] No se pudo guardar el resultado de la PC en ranking.csv");
    }

    // ¿Jugar de nuevo?
    printf("\n¿Jugar otra partida JvPC? (s/n): ");
    int ch = getchar();
    int dump; while ((dump = getchar()) != '\n' && dump != EOF) {}
    if (ch == 's' || ch == 'S') {
        playPVC(); // simple y suficiente para este proyecto
    }
}