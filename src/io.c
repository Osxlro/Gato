#include "io.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>


#include "io.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static int ieq(const char *a, const char *b) { /* case-insensitive */
    for (; *a && *b; ++a, ++b) {
        char ca = (char)tolower((unsigned char)*a);
        char cb = (char)tolower((unsigned char)*b);
        if (ca != cb) return 0;
    }
    return *a == '\0' && *b == '\0';
}

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
            int ch; while ((ch = fgetc(f)) != '\n' && ch != EOF) {}
        }
    }
    fclose(f);
    return n;
}

int saveAllRanking(const PlayerRecord arr[], int n, const char *filePath) {
    FILE *f = fopen(filePath ? filePath : "ranking.csv", "w");
    if (!f) return 0;
    for (int i = 0; i < n; ++i) {
        if (fprintf(f, "%s,%d,%d,%d,%d\n",
                    arr[i].name, arr[i].wins, arr[i].draws,
                    arr[i].losses, arr[i].score) <= 0) {
            fclose(f);
            return 0;
        }
    }
    fclose(f);
    return 1;
}

int upsertResult(const char *name, int wins, int draws, int losses, int score) {
    
    if (!name || !*name) return 0;

    PlayerRecord recs[2000];
    int n = loadRanking(recs, 2000, "ranking.csv");
    if (n < 0) n = 0; /* archivo no existe aun */

    int idx = -1;
    for (int i = 0; i < n; ++i) {
        if (ieq(recs[i].name, name)) { idx = i; break; }
    }
    if (idx < 0) { /* nuevo */
        if (n >= 2000) return 0;
        strncpy(recs[n].name, name, NAME_MAX - 1);
        recs[n].name[NAME_MAX - 1] = '\0';
        recs[n].wins   = wins;
        recs[n].draws  = draws;
        recs[n].losses = losses;
        recs[n].score  = score;
        n++;
    } else { /* actualizar acumulando */
        recs[idx].wins   += wins;
        recs[idx].draws  += draws;
        recs[idx].losses += losses;
        recs[idx].score  += score;
    }
    return saveAllRanking(recs, n, "ranking.csv");
}

void sortRankingDesc(PlayerRecord arr[], int n) {
    // Ordena por puntaje desc; desempate por nombre asc
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j + 1 < n; ++j) {
            int swap = 0;
            if (arr[j].score < arr[j+1].score) {
                swap = 1;
            } else if (arr[j].score == arr[j+1].score) {
                if (strcmp(arr[j].name, arr[j+1].name) > 0) swap = 1;
            }
            if (swap) {
                PlayerRecord tmp = arr[j];
                arr[j]   = arr[j+1];
                arr[j+1] = tmp;
            }
        }
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