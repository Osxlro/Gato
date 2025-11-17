/*
 * app.c
 *
 * Responsabilidades:
 * - Implementar el bucle principal del menú de la aplicación (mainMenuLoop).
 * - Gestionar la navegación del usuario entre las diferentes pantallas
 * (Jugar, Ranking, Ayuda, Salir).
 * - Solicitar al usuario la selección del modo de juego (JvJ o JvPC).
 * - Invocar las funciones correspondientes de otros módulos (playPVP,
 * playPVC, loadRanking, showRanking, showHelp).
 *
 * Notas:
 * - Este archivo actúa como el "controlador" principal o el "director de
 * orquesta" de la aplicación, decidiendo qué módulo debe actuar.
 * - La lógica de manejo de entrada (scanf/getchar) es específica para
 * la navegación del menú, no para el juego en sí.
 *
 * Posibles bugs:
 * - La lectura de opciones (con 'scanf("%d", &option)' y 'getchar()'
 * para el submenú 'm') es frágil. Si el usuario introduce "1a" o "a",
 * 'scanf' fallará (dejando "a\n" en el búfer) y 'getchar' leerá la 'a',
 * provocando un bucle infinito de "Entrada inválida" o comportamiento
 * inesperado.
 */

 // Librerias del Juego
#include "app.h"
#include "ui.h"
#include "game.h"
#include "io.h"

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
        
        // Leer la opción numérica del menú
        if (scanf("%d", &option) != 1) {
            // Error de entrada: limpiar el búfer
            int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
            puts("\nEntrada invalida.");
            pauseEnter();
            continue;
        }
        // Limpiar el búfer de entrada (el '\n' restante)
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}

        switch (option) {
        case 1: /* Jugar */
            clearScreen();
            puts("[1] Seleccione modo:");
            puts("a) Jugador vs Jugador");
            puts("b) Jugador vs PC");
            puts("c) Volver");
            printf("Opcion: ");
            char m = getchar(); // Leer la opción del submenú
            // Limpiar el búfer de entrada
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            
            if (m == 'a' || m == 'A') {
                playPVP(); // Iniciar modo JvJ
            } else if (m == 'b' || m == 'B') {
                playPVC(); // Iniciar modo JvPC
            } else if (m == 'c' || m == 'B') {
                pauseEnter();
                break;
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