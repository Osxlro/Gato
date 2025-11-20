/*
 * app.c
 *
 * Responsabilidades:
 * - Implementar el bucle principal del menú de la aplicación (mainMenuLoop).
 * - Gestionar la navegación del usuario entre las diferentes pantallas
 *   (Jugar, Ranking, Ayuda, Salir).
 * - Solicitar al usuario la selección del modo de juego (JvJ, JvPC y JvJ Online).
 * - Invocar las funciones correspondientes de otros módulos (playPVP,
 * playPVC, playOnline, loadRanking, showRanking, showHelp).
 *
 * Notas:
 * - Este archivo actúa como el "controlador" principal o el "director de
 * orquesta" de la aplicación, decidiendo qué módulo debe actuar.
 *
 */

 // Librerias del Juego
#include "app.h"    // Prototipos de funciones
#include "ui.h"     // Para clearScreen(), showMainMenu(), pauseEnter()
#include "game.h"   // Para playPVP(), playPVC()
#include "io.h"     // Para loadRanking(), sortRankingDesc(), showRanking()
#include "utils.h"  // Para readIn()

// Librerias del Lenguaje
#include <stdio.h>
#include <string.h>

// Bucle principal que muestra el menú y maneja la selección del usuario.
int mainMenuLoop(void) {
    int running = 1;
    while (running) {
        clearScreen();
        showMainMenu();
        int option = 0;
        char tempBuffer[100];

        if (!readIn(tempBuffer, sizeof(tempBuffer))) {
            running = 0; // EOF o error, salir
            continue;
        }

        if (sscanf(tempBuffer, "%d",&option) != 1) {
            puts("\nEntrada Invalida");
            pauseEnter();
            continue;
        }

        switch (option) {
        case 1: /* Jugar */
            clearScreen();
            puts("[1] Seleccione modo:");
            puts("a) Jugador vs Jugador");
            puts("b) Jugador vs PC");
            puts("c) Jugador vs Jugador Online");
            puts("d) Volver");
            printf("Opcion: ");

            // Leer la opción del submenú
            char m = '\0';
            readIn(tempBuffer, sizeof(tempBuffer));
            if (strlen(tempBuffer) > 0) {
                m = tempBuffer[0];
            }
            
            if (m == 'a' || m == 'A') {
                playPVP(); // Iniciar modo JvJ
            } else if (m == 'b' || m == 'B') {
                playPVC(); // Iniciar modo JvPC
            } else if (m == 'd' || m == 'D') {
                pauseEnter(); 
                break;
            } else if (m == 'c' || m == 'C') {
                playOnline(); // Iniciar modo JvJ Online
            } else {
                puts("Opcion invalida.");
            }
            pauseEnter();
            break;

        case 2: { /* Ranking */
            clearScreen();
            PlayerRecord recs[1000]; // Array para almacenar los registros
            // Cargar el ranking desde el archivo
            int n = loadRanking(recs, 1000, "ranking.csv");
            if (n < 0) {
                puts("No hay Ranking.");
            } else {
                sortRankingDesc(recs, n); // Ordenar
                showRanking(recs, n);     // Mostrar
            }
            pauseEnter();
            break;
        }

        case 3: /* Ayuda */
            clearScreen();
            showHelp();
            pauseEnter();
            break;

        case 4: /* Salir */
            running = 0; // Terminar el bucle
            break;

        default:
            puts("Opcion invalida.");
            pauseEnter();
            break;
        }
    }
    return 0; // Salir del programa
}