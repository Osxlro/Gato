#include "io.h"
#include <stdio.h>
#include <string.h>

int saveResult(const char *name, int wins, int draws, int losses, int score) {
    if (!name || !*name) return 0;
    FILE *f = fopen("ranking.csv", "a");
    if (!f) return 0;
    int ok = fprintf(f, "%s,%d,%d,%d,%d\n", name, wins, draws, losses, score) > 0;
    fclose(f);
    return ok;
}

int loadRanking(PlayerRecord arr[], int max, const char *filePath) {
    FILE *f = fopen(filePath ? filePath : "ranking.csv", "r");
    if (!f) return -1;
    int n = 0;
    while (n < max && !feof(f)) {
        PlayerRecord pr;
        if (fscanf(f, "%31[^,],%d,%d,%d,%d\n",
                   pr.name, &pr.wins, &pr.draws, &pr.losses, &pr.score) == 5) {
            arr[n++] = pr;
        } else {
            // línea malformada: descartar
            int ch;
            while ((ch = fgetc(f)) != '\n' && ch != EOF) {}
        }
    }
    fclose(f);
    return n;
}

void sortRankingDesc(PlayerRecord arr[], int n) {
    // bubble sort simple (suficiente para n pequeño)
    for (int i = 0; i < n; ++i)
        for (int j = 0; j+1 < n; ++j)
            if (arr[j].score < arr[j+1].score) {
                PlayerRecord tmp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = tmp;
            }
}

void showRanking(const PlayerRecord arr[], int n) {
    puts("+----+------------------------------+---+---+---+--------+");
    puts("| #  | Nombre                       | G | E | P | Puntaje|");
    puts("+----+------------------------------+---+---+---+--------+");
    for (int i = 0; i < n; ++i) {
        printf("| %-2d | %-28s | %-1d | %-1d | %-1d | %-6d|\n",
            i+1, arr[i].name, arr[i].wins, arr[i].draws, arr[i].losses, arr[i].score);
    }
    puts("+----+------------------------------+---+---+---+--------+");
}