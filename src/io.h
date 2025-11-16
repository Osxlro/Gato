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

#endif