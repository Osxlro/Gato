/*
 * game.c
 *
 * Responsabilidades:
 * - Implementar los bucles de juego principales: Jugador vs Jugador (playPVP)
 * y Jugador vs PC (playPVC).
 * - Gestionar el estado del tablero (initBoard, applyMove, isCellEmpty).
 * - Implementar la lógica de turnos y la asignación de símbolos ('X'/'O').
 * - Determinar el resultado de la partida (checkWin, boardFull).
 * - Solicitar y validar la entrada de movimientos del usuario (readMove, isValidCell).
 * - Persistir los resultados al final de cada partida (llamando a upsertResult de io.h).
 * - Manejar la generación de números aleatorios para el jugador inicial (randomStarts).
 * - Contener funciones auxiliares para la lógica del juego (scoreOf).
 *
 * Notas:
 * - Este es el "motor" central del juego.
 * - El estado del juego (tablero, nombres) se maneja como variables locales
 * dentro de las funciones de bucle de juego, no hay variables globales.
 *
 * Posibles bugs:
 * - Sin bugs conocidos.
 */

// Librerias del juego
#include "game.h"  // Prototipos de funciones y definiciones
#include "ui.h"    // Para clearScreen()
#include "io.h"    // Para upsertResult()
#include "ai.h"    // Para AI_PLAYER_NAME
#include "utils.h" // Para readIn()
#include "network.h" // Para funciones de red

// Librerias por el Lenguaje
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifndef NAME_MAX
#define NAME_MAX 32
#endif

/* Funciones básicas (Solicitar Nombres) (Semilla de Aleatoriedad) */
// Solicitar nombres.
void askPlayerNames(char p1[], char p2[], int maxLen) { // PVP - Jugador vs Jugador
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

void askHumanName(char p1[], int maxLen) {              // PVC - Jugador vs PC  
    clearScreen();

    puts("--- MODO JvPC ---");
    printf("Tu nombre: ");
    readIn(p1, maxLen);
    sanitizeString(p1);
    if (p1[0] == '\0') strncpy(p1, "Humano", maxLen);
}

// Semilla para aleatoriedad (inicializar solo una vez)
static int seeded = 0;
static void ensureSeed(void) {
    if (!seeded) { seeded = 1; srand((unsigned)time(NULL)); }
}

// Devuelve 0 o 1 aleatoriamente para decidir quién inicia.
int randomStarts(void) { ensureSeed(); return rand() & 1; } /* 0 o 1 */

/* ---------- JvJ con inicio aleatorio ---------- */
void playPVP(void) {
    char name1[NAME_MAX], name2[NAME_MAX];
    askPlayerNames(name1, name2, NAME_MAX);

    char board[3][3];
    initBoard(board);

    /* Quien inicia lleva 'X' */
    int starter = randomStarts();      /* 0 -> nombre1 empieza, 1 -> nombre2 empieza */
    int current = starter;             /* 0 o 1 */
    int r, c;

    int wins[2]  = {0,0};
    int draws[2] = {0,0};
    int losses[2]= {0,0};

    for (;;) { // Bucle infinito hasta que alguien gane o empate.
        clearScreen();
        printf("Modo: Jugador vs Jugador\n");
        printf("Inicia: %s \n", starter==0 ? name1 : name2);
        printf("Turno:  %s (%c)\n",
               current==0 ? name1 : name2,
               (current==starter) ? 'X' : 'O');
        printBoard(board);

        printf("Ingresa fila y columna ([1 3]..[2 1]..) o 'q' para salir: ");
        
        // Lee el movimiento (0 = error, -1 = salir, 1 = éxito)
        int moveResult = readMove(&r,&c);
        if (moveResult == 0) {          // Error
            puts("Entrada Invalida.");
            pauseEnter();
            continue;

        } else if (moveResult == -1) {  // Usuario quiere salir
            clearScreen();
            printBoard(board);

            const char* quitterName = (current == 0) ? name1 : name2;
            const char* winnerName = (current == 0) ? name2 : name1;
            char sym = (current==starter) ? 'X' : 'O';

            printf("\n%s (%c) ha abandonado la partida.\n", quitterName, sym);
            printf("Gana %s!\n", winnerName);

            wins[1-current]++; // El oponente gana
            losses[current]++; // El jugador actual pierde
            break; // Salir del bucle for(;;)

        }
        
        // Validar movimiento
        if (!isValidCell(r, c)) { puts("Fuera de rango."); pauseEnter(); continue; }
        if (!isCellEmpty(board, r, c)) { puts("Casilla ocupada."); pauseEnter(); continue; }

        char sym = (current==starter) ? 'X' : 'O';
        applyMove(board, r, c, sym);

        // Verificar Victoria o Empate
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
        puts("\n[ADVERTENCIA] No se pudo actualizar Score para Jugador 1.");
    if (!upsertResult(name2, wins[1], draws[1], losses[1], score1))
        puts("[ADVERTENCIA] No se pudo actualizar Score para Jugador 2.");

    printf("\nJugar otra partida JvJ? (s/n): ");
    int ch = getchar(); int dump; while ((dump=getchar())!='\n' && dump!=EOF){}
    if (ch=='s'||ch=='S') playPVP();
}

/* ---------- JvPC con inicio aleatorio ---------- */
extern void pcMove(char board[3][3], char pcSym, char humanSym);
void playPVC(void) {
    char human[NAME_MAX];
    const char pcName[] = AI_PLAYER_NAME;
    askHumanName(human, NAME_MAX);

    char board[3][3];
    initBoard(board);

    int starter = randomStarts(); /* 0 -> Jugador inicia (X), 1 -> PC inicia (X) */
    int current = starter;

    int winsH=0, drawsH=0, lossesH=0;
    int winsPC=0, drawsPC=0, lossesPC=0;

    int r,c;
    for (;;) { // Bucle infinito hasta que alguien gane o empate.
        clearScreen();
        printf("Modo: Jugador vs PC\n");
        printf("Inicia: %s \n", starter==0 ? human : pcName);
        printf("Turno:  %s (%c)\n", current==0 ? human : pcName,
               (current==starter) ? 'X' : 'O');
        printBoard(board);

        if (current == 0) { /* Jugador */
            printf("Ingresa fila y columna ([1 3]..[2 1]..) o 'q' para salir: ");

            // Lee el movimiento (0 = error, -1 = salir, 1 = éxito)
            int moveResult = readMove(&r,&c);
            if (moveResult == 0) { puts("Entrada invalida."); pauseEnter(); continue; }
            if (moveResult == -1) { 
                clearScreen();
                printBoard(board);
                printf("\n%s (%c) ha abandonado la partida.\n",
                       human,
                       (current==starter) ? 'X' : 'O');
                printf("Gana %s!\n", pcName);
                winsPC++; lossesH++;
                break;
             }

            // Validar movimiento
            if (!isValidCell(r,c)) { puts("Fuera de rango."); pauseEnter(); continue; }
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
        puts("\n[ADVERTENCIA] No se pudo actualizar ranking para Jugador.");
    if (!upsertResult(pcName, winsPC, drawsPC, lossesPC, scorePC))
        puts("[ADVERTENCIA] No se pudo actualizar ranking para PC.");

    printf("\nJugar otra partida JvPC? (s/n): ");
    int ch = getchar(); int dump; while ((dump=getchar())!='\n' && dump!=EOF){}
    if (ch=='s'||ch=='S') playPVC();
}

/* ---------- JvJ Online ---------- */
void playOnline(void) {
    // 1. Inicializar Winsock
    if (!net_init()) {
        puts("Error inicializando red.");
        pauseEnter();
        return;
    }

    clearScreen();
    puts("=== MODO ONLINE (LAN) ===");
    
    char myName[NAME_MAX];
    printf("Tu nombre para Online: ");

    // 2. Leer Nombre del Jugador
    readIn(myName, NAME_MAX);
    sanitizeString(myName);
    if (myName[0] == '\0') strcpy(myName, "Jugador");

    // Variables de conexión
    int socket_fd = -1;
    int amIHost = 0; 
    int port = GAME_PORT;

    // 3. Bucle de Conexión
    while (socket_fd < 0) {
        clearScreen();

        puts("=== MODO ONLINE (LAN) ===");
        printf("Jugador: %s\n", myName);
        puts("---------------------------");
        puts("1. Crear Partida   (Ser Host)");
        puts("2. Buscar Partida  (Auto-descubrimiento)");
        puts("3. Conectar Manual (IP especifica)");
        puts("4. Volver");
        printf("Opcion: ");

        char buffer[10];
        readIn(buffer, sizeof(buffer));
        int option = atoi(buffer);
        
        switch (option) {
        case 1: /* HOST */
            amIHost = 1;
            socket_fd = net_host_wait_for_client(port);
            if (socket_fd < 0) {
                puts("Tiempo de espera agotado o error del servidor.");
                pauseEnter();
            }
            break;

        case 2: /* CLIENTE (AUTO) */
            amIHost = 0;
            {
                char ip[32];
                if (net_discover_host(port, DISCOVERY_TIMEOUT, ip)) {
                    socket_fd = net_connect_to_host(ip, port);
                } else {
                    printf("\nNo se encontro Host en la red local.\n");
                    pauseEnter();
                }
            }
            break;

        case 3: /* CLIENTE (MANUAL) */
            amIHost = 0;
            {
                char ip[32];
                printf("\nIngresa la IP del Host: ");
                readIn(ip, sizeof(ip));
                socket_fd = net_connect_to_host(ip, port);

                if (socket_fd < 0) {
                    // Registra error de parte de network y de aqui mostramos otro mensaje
                    puts("No se pudo conectar al Host.");
                    pauseEnter();
                }
            }
            break;

        case 4: /* VOLVER */
            net_cleanup();
            return;
        
        default:
            puts("Opcion invalida.");
            pauseEnter();
            break; 
        }
    }

    // 4. Handshake (Intercambio de Nombres)
    printf("\nConectado! Intercambiando datos...\n");
    char rivalName[NAME_MAX];
    
    net_send_name(socket_fd, myName);
    if (!net_receive_name(socket_fd, rivalName)) {
        puts("Error de protocolo (handshake).");
        net_close(socket_fd);
        net_cleanup();
        pauseEnter();
        return;
    }
    if (rivalName[0] == '\0') strcpy(rivalName, "Rival");

    // 5. Bucle de Partidas (Rematch Loop)
    int playing = 1;
    while (playing) {
        char board[3][3];
        initBoard(board);
        
        // Configuración fija: Host=X, Cliente=O
        char mySym    = amIHost ? 'X' : 'O';
        char rivalSym = amIHost ? 'O' : 'X';
        
        // El Host (X) siempre inicia el turno
        int current = 0; // 0=X, 1=O
        int r, c;
        int gameEnded = 0; // Bandera para saber si la partida terminó

        // --- Bucle de Turnos ---
        for (;;) {
            clearScreen();
            printf("Modo: Online\n");
            printf("------------------------------\n");
            printf(" TU:    %s (%c)\n", myName, mySym);
            printf(" RIVAL: %s (%c)\n", rivalName, rivalSym);
            printf("------------------------------\n");
            printBoard(board);

            // ¿Es mi turno?
            int isMyTurn = (current == 0 && amIHost) || (current == 1 && !amIHost);

            if (isMyTurn) {
                printf("\n[TU TURNO] Ingresa fila y columna (o 'q' para rendirse): ");
                int res = readMove(&r, &c);

                if (res == -1) { // 'q' presionada
                    net_send_move(socket_fd, -1, -1); // Avisar rendición
                    printf("\nTe has rendido. Gana %s.\n", rivalName);
                    // (Opcional: upsertResult para registrar derrota)
                    gameEnded = 1;
                    break;
                }
                
                if (res == 0 || !isValidCell(r, c) || !isCellEmpty(board, r, c)) {
                    puts("Movimiento invalido.");
                    pauseEnter();
                    continue;
                }

                // Aplicar y enviar
                applyMove(board, r, c, mySym);
                net_send_move(socket_fd, r, c);

                if (checkWin(board, mySym)) {
                    clearScreen(); printBoard(board);
                    printf("\nFELICIDADES %s HAS GANADO.\n", myName);
                    // (Opcional: upsertResult para registrar victoria)
                    gameEnded = 1;
                    break;
                }

            } else {
                printf("\n[ESPERANDO A %s]...\n", rivalName);
                
                // Bloquea hasta recibir
                if (!net_receive_move(socket_fd, &r, &c)) {
                    puts("\nError: Se perdio la conexion con el rival.");
                    playing = 0; // Salir del bucle principal
                    gameEnded = 1; // Salir del bucle de juego
                    break; 
                }

                // Chequear rendición del rival
                if (r == -1) {
                    printf("\n%s SE HA RENDIDO, GANASTE.\n", rivalName);
                    // (Opcional: upsertResult para registrar victoria)
                    gameEnded = 1;
                    break;
                }

                applyMove(board, r, c, rivalSym);

                if (checkWin(board, rivalSym)) {
                    clearScreen(); printBoard(board);
                    printf("\n%s te ha ganado. Suerte la proxima.\n", rivalName);
                    // (Opcional: upsertResult para registrar derrota)
                    gameEnded = 1;
                    break;
                }
            }

            if (boardFull(board)) {
                clearScreen(); printBoard(board);
                puts("\nEMPATE Partida reñida.");
                gameEnded = 1;
                break;
            }

            current = 1 - current; // Cambio de turno
        }

        // Si se perdió la conexión abruptamente, salimos
        if (playing == 0) break;

        // 6. Votación de Revancha
        if (gameEnded) {
            printf("\nJugar de nuevo contra %s? (s/n): ", rivalName);
            char vote[10];
            readIn(vote, sizeof(vote));
            int my_vote = (vote[0] == 's' || vote[0] == 'S');

            printf("Esperando respuesta de %s...\n", rivalName);
            
            if (net_negotiate_rematch(socket_fd, my_vote)) {
                printf("Ambos aceptaron, Reiniciando...\n");
                // El bucle 'while(playing)' continúa, se reinicia el tablero
            } else {
                printf("Uno de los jugadores decidio salir.\n");
                playing = 0;
            }
        }
    }

    puts("\nCerrando conexion...");
    net_close(socket_fd);
    net_cleanup();
}

 /* Funciones de Verificación
 * (Verificar Celdas, Aplicar Movimiento, Verificar Victoria, Tablero Lleno,)
 */

// Verifica si las coordenadas (r, c) están dentro del rango 1-3.
int isValidCell(int r, int c) {
    return r >= 1 && r <= 3 && c >= 1 && c <= 3;
}

// Inicializa el tablero 3x3 con espacios vacíos ' '.
void initBoard(char board[3][3]) {
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            board[r][c] = ' ';
}

// Comprueba si una celda específica del tablero (coordenadas 1-3) está vacía.
int isCellEmpty(const char board[3][3], int r, int c) {
    return board[r-1][c-1] == ' ';
}

// Coloca un símbolo (sym) en la celda (r, c) del tablero.
// Devuelve 1 si tuvo éxito, 0 si la celda no era válida o estaba ocupada.
int applyMove(char board[3][3], int r, int c, char sym) {
    if (!isValidCell(r, c)) return 0;
    if (!isCellEmpty(board, r, c)) return 0;
    board[r-1][c-1] = sym;
    return 1;
}

// Verifica si el jugador con el símbolo 'sym' ha ganado (3 en línea).
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

// Comprueba si el tablero está lleno (no quedan espacios ' ').
int boardFull(const char board[3][3]) {
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (board[r][c] == ' ') return 0;
    return 1;
}

// Lee la entrada del usuario para fila y columna (ej: "1 2") y limpia el búfer.
int readMove(int *r, int *c) {
    char tempBuffer[100];

    // 1. Leer la linea completa.
    if(!readIn(tempBuffer, sizeof(tempBuffer))) {
        return 0; // Error o EOF
    }

    // 2. Verificar si el usuario quiere salir ('q' o 'Q').
    if ((tolower((unsigned char)tempBuffer[0]) == 'q') && tempBuffer[1] == '\0') {
        return -1; // Devolvemos -1 como señal de "salir"
    }
    
    // 2. Parsear datos desde el buffer.
    if (sscanf(tempBuffer, "%d %d", r, c) != 2) {
        return 0; // Fallo al parsear dos enteros
    }

    return 1; // Lectura exitosa
}

// Calcula el puntaje basado en victorias (3 pts) y empates (1 pto).
int scoreOf(int wins, int draws, int losses) {
    (void)losses;
    return 3*wins + 1*draws;
}