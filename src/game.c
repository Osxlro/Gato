/*
 * Nombre del archivo: game.c
 *
 * Responsabilidades:
 * - Implementar los flujos principales de juego: Local (PvP), Contra IA (PvPC) y En Red (Online).
 * - Centralizar la validación de movimientos (getValidMove) para evitar código repetido.
 * - Centralizar la verificación de fin de juego (checkAndPrintEnd) para victorias y empates.
 * - Gestionar la entrada de nombres de jugadores y su sanitización.
 * - Coordinar la lógica de red (Handshake, Sincronización de turnos, Revancha) en el modo Online.
 * - Persistir los resultados en el ranking local (llamando a upsertResult de io.c).
 *
 */

// Librerias del juego
#include "game.h"  
#include "ui.h"    
#include "io.h"    
#include "ai.h"    
#include "utils.h" 
#include "network.h" 

// Librerias por el Lenguaje
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifndef NAME_MAX
#define NAME_MAX 32
#endif

/* --- FUNCIONES PRIVADAS --- */

// Semilla para aleatoriedad
static int seeded = 0;
static void ensureSeed(void) {
    if (!seeded) { seeded = 1; srand((unsigned)time(NULL)); }
}

static int readMove(int *r, int *c);
static int scoreOf(int wins, int draws, int losses);

/* *Solicita movimiento.
 * Recibe 'header' para redibujar el título si hay que limpiar la pantalla por error.
 */
static int getValidMove(const char board[3][3], int *r, int *c, const char* header) {
    printf("\nIngresa fila y columna (o 'q' para salir): ");

    while (1) {
        int res = readMove(r, c);
        if (res == -1) return -1; // Rendición
        
        // Detectar tipo de error
        int error = 0;
        if (res == 0) error = 1;                            // Formato
        else if (!isValidCell(*r, *c)) error = 2;           // Rango
        else if (!isCellEmpty(board, *r, *c)) error = 3;    // Ocupado

        if (error) {
            if (error == 1) puts(">> Formato invalido. Usa: fila columna (ej. 1 2)");
            if (error == 2) puts(">> Coordenadas fuera de rango (1-3).");
            if (error == 3) puts(">> Esa casilla ya esta ocupada.");
            
            pauseEnter(); // Pausa para leer el error

            // LIMPIEZA Y REDIBUJADO
            clearScreen();
            if (header) puts(header); // Restaurar el título
            printBoard(board);        // Restaurar el tablero

            printf("\nIntenta de nuevo ([1 2].. [3 1]..): ");
            continue; 
        }

        return 1; // Movimiento válido
    }
}

/*
 * Verifica fin de juego.
 * Imprime mensajes de Victoria o Empate.
 * Retorna: 1 si terminó, 0 si sigue.
 */
static int checkAndPrintEnd(const char board[3][3], char sym, const char* winnerName) {
    if (checkWin(board, sym)) {
        clearScreen(); 
        printBoard(board);
        printf("\n¡GANASTE %s (%c)!\n", winnerName, sym);
        return 1; // Victoria
    }
    if (boardFull(board)) {
        clearScreen(); 
        printBoard(board);
        puts("\n¡EMPATE! Tablero lleno.");
        return 1; // Empate
    }
    return 0; // Sigue jugando
}

/* --- FUNCIONES DE UI (NOMBRES) --- */

void askPlayerNames(char p1[], char p2[], int maxLen) { // JUGADOR VS JUGADOR
    clearScreen();
    puts("--- MODO JvJ ---");
    printf("Nombre Jugador 1: ");
    readIn(p1, maxLen);
    sanitizeString(p1);
    if (p1[0] == '\0') strncpy(p1, "Jugador1", maxLen);

    printf("Nombre Jugador 2: ");
    readIn(p2, maxLen);
    sanitizeString(p2);
    if (p2[0] == '\0') strncpy(p2, "Jugador2", maxLen);
}

void askHumanName(char p1[], int maxLen) { // JUGADOR VS PC
    clearScreen();
    puts("--- MODO JvPC ---");
    printf("Tu nombre: ");
    readIn(p1, maxLen);
    sanitizeString(p1);
    if (p1[0] == '\0') strncpy(p1, "Humano", maxLen);
}

int randomStarts(void) { ensureSeed(); return rand() & 1; }

/* --- MODOS DE JUEGO --- */

/* 1. Jugador vs Jugador (Local) */
void playPVP(void) {
    char name1[NAME_MAX], name2[NAME_MAX];
    askPlayerNames(name1, name2, NAME_MAX);

    char board[3][3];
    initBoard(board);

    int starter = randomStarts();
    int current = starter;
    int r, c;
    int wins[2] = {0,0}, draws[2] = {0,0}, losses[2] = {0,0};

    // Bucle de juego
    for (;;) {
        char headerStr[128];
        snprintf(headerStr, sizeof(headerStr), "Modo: PvP | Turno: %s (%c)", 
                 current==0 ? name1 : name2, (current==starter) ? 'X' : 'O');
        
        clearScreen();
        puts(headerStr);
        printBoard(board);

        if (getValidMove(board, &r, &c, headerStr) == -1) {
            const char* winner = (current == 0) ? name2 : name1;
            const char* loser  = (current == 0) ? name1 : name2;
            printf("\n%s se ha rendido. Gana %s!\n", loser, winner);
            wins[1-current]++; losses[current]++;
            break;
        }

        char sym = (current==starter) ? 'X' : 'O';
        applyMove(board, r, c, sym);

        if (checkAndPrintEnd(board, sym, current==0 ? name1 : name2)) {
            if (checkWin(board, sym)) { wins[current]++; losses[1-current]++; }
            else { draws[0]++; draws[1]++; }
            break;
        }
        
        current = 1 - current;
    }

    int s0 = scoreOf(wins[0], draws[0], losses[0]);
    int s1 = scoreOf(wins[1], draws[1], losses[1]);
    upsertResult(name1, wins[0], draws[0], losses[0], s0);
    upsertResult(name2, wins[1], draws[1], losses[1], s1);

    printf("\nJugar otra partida JvJ? (s/n): ");
    char resp[10]; readIn(resp, 10);
    if (resp[0]=='s'||resp[0]=='S') playPVP();
    
}

/* 2. Jugador vs PC */
void playPVC(void) {
    char human[NAME_MAX];
    const char pcName[] = AI_PLAYER_NAME;
    askHumanName(human, NAME_MAX);

    char board[3][3];
    initBoard(board);

    int starter = randomStarts();
    int current = starter;
    int r, c;
    int winsH=0, lossesH=0, winsPC=0, lossesPC=0, draws=0;

    for (;;) {
        char headerStr[128];
        snprintf(headerStr, sizeof(headerStr), "Modo: PvPC | Turno: %s", 
                 current==0 ? human : pcName);

        clearScreen();
        puts(headerStr);
        printBoard(board);

        char sym = (current == starter) ? 'X' : 'O';

        if (current == 0) { /* Turno Humano */
            // Pasamos headerStr
            if (getValidMove(board, &r, &c, headerStr) == -1) {
                printf("\nTe has rendido. Gana %s.\n", pcName);
                winsPC++; lossesH++;
                break;
            }
            applyMove(board, r, c, sym);

        } else { /* Turno PC */
            pcMove(board, sym, (sym=='X') ? 'O' : 'X');
        }

        const char* currentName = (current==0) ? human : pcName;
        
        if (checkAndPrintEnd(board, sym, currentName)) {
            if (checkWin(board, sym)) {
                if (current==0) { winsH++; lossesPC++; } 
                else { winsPC++; lossesH++; }
            } else {
                draws++;
            }
            break;
        }

        current = 1 - current;
    }

    upsertResult(human, winsH, draws, lossesH, scoreOf(winsH, draws, lossesH));
    upsertResult(pcName, winsPC, draws, lossesPC, scoreOf(winsPC, draws, lossesPC));

    printf("\nJugar otra partida JvPC? (s/n): ");
    char resp[10]; readIn(resp, 10);
    if (resp[0]=='s'||resp[0]=='S') playPVC();
}

/* 3. Online (LAN) */
void playOnline(void) {
    if (!net_init()) { puts("Error de red."); return; }

    clearScreen();
    puts("--- MODO ONLINE (LAN) ---");
    char myName[NAME_MAX];
    printf("Tu nombre: ");
    readIn(myName, NAME_MAX);
    sanitizeString(myName);
    if (myName[0] == '\0') strcpy(myName, "Jugador");

    int socket_fd = -1;
    int amIHost = 0;
    int port = GAME_PORT;

    // Menú de Conexión
    while (socket_fd < 0) {
        clearScreen();
        printf("Jugador: %s\n\n", myName);
        puts("1. Crear Partida (Host)");
        puts("2. Buscar Partida (Auto)");
        puts("3. Conectar Manual (IP)");
        puts("4. Volver");
        printf("Opcion: ");

        char buf[10]; readIn(buf, 10);
        int opt = atoi(buf);

        switch (opt) {
            case 1: 
                amIHost = 1; 
                socket_fd = net_host_wait_for_client(port); 
                if(socket_fd<0) pauseEnter();
                break;
            case 2: {
                amIHost = 0; char ip[32];
                if (net_discover_host(port, DISCOVERY_TIMEOUT, ip)) 
                    socket_fd = net_connect_to_host(ip, port);
                else { puts("No encontrado."); pauseEnter(); }
                break;
            }
            case 3: {
                amIHost = 0; char ip[32];
                printf("IP Host: "); readIn(ip, 32);
                printf("Conectando...");
                socket_fd = net_connect_to_host(ip, port);
                if(socket_fd<0) pauseEnter();
                break;
            }
            case 4: net_cleanup(); return;
            default: puts("Invalido."); pauseEnter(); break;
        }
    }

    // Handshake
    printf("\nConectado! Intercambiando nombres...\n");
    char rivalName[NAME_MAX];
    net_send_name(socket_fd, myName);
    if (!net_receive_name(socket_fd, rivalName)) {
        puts("Error handshake."); net_close(socket_fd); net_cleanup(); return;
    }
    if (!rivalName[0]) strcpy(rivalName, "Rival");

    // Bucle Partidas
    int playing = 1;
    int starter_offset = 0;

    while (playing) {
        char board[3][3]; initBoard(board);
        char mySym = amIHost ? 'X' : 'O';
        char rivalSym = amIHost ? 'O' : 'X';
        int current = starter_offset;
        int gameEnded = 0;

for (;;) {
            char headerStr[128];
            snprintf(headerStr, sizeof(headerStr), 
                     "Modo: Online: %s (%c) vs %s (%c)", myName, mySym, rivalName, rivalSym);

            clearScreen();
            puts(headerStr);
            printBoard(board);

            int isMyTurn = (current == 0 && amIHost) || (current == 1 && !amIHost);

            if (isMyTurn) {
                int r, c;
                int res = getValidMove(board, &r, &c, headerStr);

                if (res == -1) { 
                    gameEnded = 1; break;
                }
                
                applyMove(board, r, c, mySym);
                net_send_move(socket_fd, r, c);

                if (checkWin(board, mySym)) {
                    clearScreen(); printBoard(board);
                    printf("\nGANASTE!\n");
                    upsertResult(myName, 1, 0, 0, 3);
                    gameEnded = 1; break;
                }

            } else {
                printf("\n[ESPERANDO A %s]...\n", rivalName);
                int r, c;
                if (!net_receive_move(socket_fd, &r, &c)) {
                    puts("Desconectado."); playing=0; gameEnded=1; break;
                }
                if (r == -1) {
                    printf("\n%s SE RINDIO. GANASTE.\n", rivalName);
                    upsertResult(myName, 1, 0, 0, 3);
                    gameEnded=1; break;
                }
                applyMove(board, r, c, rivalSym);
                if (checkWin(board, rivalSym)) {
                    clearScreen(); printBoard(board);
                    printf("\nPerdiste.\n");
                    upsertResult(myName, 0, 0, 1, 0);
                    gameEnded=1; break;
                }
            }

            if (boardFull(board)) {
                clearScreen(); printBoard(board);
                puts("\nEMPATE");
                // Guardar empate
                upsertResult(myName, 0, 1, 0, 1);
                gameEnded = 1; break;
            }
            current = 1 - current;
        }

        if (!playing) break;

        if (gameEnded) {
            printf("\nRevancha? (s/n): ");
            char buf[10]; readIn(buf, 10);
            int vote = (buf[0]=='s'||buf[0]=='S');
            printf("Esperando al rival...\n");
            
            if (net_negotiate_rematch(socket_fd, vote)) {
                starter_offset = 1 - starter_offset; // Alternar turno
            } else {
                printf("Fin del juego.\n");
                playing = 0;
            }
        }
    }
    net_close(socket_fd);
    net_cleanup();
}


/* --- LOGICA CORE --- */

int isValidCell(int r, int c) { return r >= 1 && r <= 3 && c >= 1 && c <= 3; }

void initBoard(char board[3][3]) {
    for (int r=0;r<3;r++) for(int c=0;c<3;c++) board[r][c]=' ';
}

int isCellEmpty(const char board[3][3], int r, int c) {
    return board[r-1][c-1] == ' ';
}

int applyMove(char board[3][3], int r, int c, char sym) {
    if (!isValidCell(r, c) || !isCellEmpty(board, r, c)) return 0;
    board[r-1][c-1] = sym;
    return 1;
}

int checkWin(const char board[3][3], char sym) {
    for (int i=0; i<3; ++i) {
        if (board[i][0]==sym && board[i][1]==sym && board[i][2]==sym) return 1;
        if (board[0][i]==sym && board[1][i]==sym && board[2][i]==sym) return 1;
    }
    if (board[0][0]==sym && board[1][1]==sym && board[2][2]==sym) return 1;
    if (board[0][2]==sym && board[1][1]==sym && board[2][0]==sym) return 1;
    return 0;
}

int boardFull(const char board[3][3]) {
    for(int r=0;r<3;r++) for(int c=0;c<3;c++) if(board[r][c]==' ') return 0;
    return 1;
}

static int readMove(int *r, int *c) {
    char buf[100];
    if(!readIn(buf, sizeof(buf))) return 0;
    if (tolower((unsigned char)buf[0]) == 'q' && buf[1]=='\0') return -1;
    if (sscanf(buf, "%d %d", r, c) != 2) return 0;
    return 1;
}

static int scoreOf(int wins, int draws, int losses) {
    (void)losses; return 3*wins + 1*draws;
}