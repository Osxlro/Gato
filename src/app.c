#include "app.h"
#include "ui.h"
#include "game.h"
#include "io.h"
#include <stdio.h>

int mainMenuLoop(void) {
    int running = 1;
    while (running) {
        clearScreen();
        showMainMenu();
        int option = 0;
        if (scanf("%d", &option) != 1) {
            int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
            puts("\nEntrada inválida.");
            pauseEnter();
            continue;
        }
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}

        switch (option) {
        case 1: /* Jugar */
            clearScreen();
            puts("[1] Seleccione modo:");
            puts("a) Jugador vs Jugador");
            puts("b) Jugador vs PC");
            printf("Opción: ");
            char m = getchar();
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            if (m == 'a' || m == 'A') {
                playPVP();
            } else if (m == 'b' || m == 'B') {
                playPVC();
            } else {
                puts("Opción inválida.");
            }
            pauseEnter();
            break;

        case 2: { /* Ranking */
            clearScreen();
            PlayerRecord recs[1000];
            int n = loadRanking(recs, 1000, "ranking.csv");
            if (n < 0) {
                puts("No se pudo abrir ranking.csv (¿aún no existe?).");
            } else {
                sortRankingDesc(recs, n);
                showRanking(recs, n);
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
            running = 0;
            break;

        default:
            puts("Opción inválida.");
            pauseEnter();
            break;
        }
    }
    return 0;
}