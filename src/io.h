#ifndef IO_H
#define IO_H
#include <string.h>

#define NAME_MAX 32

typedef struct {
    char name[NAME_MAX];
    int  wins, draws, losses;
    int  score;
} PlayerRecord;

int saveResult(const char *name, int wins, int draws, int losses, int score);
int loadRanking(PlayerRecord arr[], int max, const char *filePath);
void sortRankingDesc(PlayerRecord arr[], int n);
void showRanking(const PlayerRecord arr[], int n);

int  upsertResult(const char *name, int wins, int draws, int losses, int score);
int  saveAllRanking(const PlayerRecord arr[], int n, const char *filePath);


void sortRankingDesc(PlayerRecord arr[], int n) {
    // Ordenamiento simple por puntaje descendente, desempate por nombre ascendente
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j + 1 < n; ++j) {
            int swap = 0;
            if (arr[j].score < arr[j+1].score) {
                swap = 1;
            } else if (arr[j].score == arr[j+1].score) {
                // desempate por nombre ascendente
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


#endif