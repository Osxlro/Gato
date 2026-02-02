/*
 * io.c
 *
 * Responsabilidades:
 * - Gestionar toda la entrada/salida (I/O) de archivos.
 * - Leer el archivo 'ranking.csv' y cargarlo en un array de PlayerRecord (loadRanking).
 * - Escribir un array de PlayerRecord de vuelta al archivo 'ranking.csv' (saveAllRanking).
 * - Proveer una función 'upsert' (update/insert) para actualizar el ranking
 * de forma segura, cargando, modificando y guardando (upsertResult).
 * - Implementar la lógica para ordenar el ranking (sortRankingDesc).
 * - Mostrar el ranking formateado en la consola (showRanking).
 *
 * Notas:
 * - Este módulo encapsula el formato CSV.
 * - La función 'upsertResult' es la interfaz principal para 'game.c'.
 * - 'saveResult' es una función antigua/simple que solo añade al final (append)
 * y no previene duplicados; 'upsertResult' es la correcta.
 * - 'ieq' es un helper local para comparar nombres sin distinción de mayúsculas.
 *
 * Posibles bugs:
 * - 'loadRanking' tiene un límite estático (p.ej. 2000) de registros.
 * Si el CSV crece más, truncará la carga.
 * 
 */

// Librerias del juego
#include "io.h"     // Prototipos de funciones y definiciones
#include "game.h"   // Para scoreOf()
#include "ai.h"     // Para AI_PLAYER_NAME

// Librerias del Lenguaje
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h> // Para qsort

// Compara dos cadenas de texto ignorando mayúsculas/minúsculas ('i' = ignore).
static int ieq(const char *a, const char *b) { /* case-insensitive */
    for (; *a && *b; ++a, ++b) {
        char ca = (char)tolower((unsigned char)*a);
        char cb = (char)tolower((unsigned char)*b);
        if (ca != cb) return 0;
    }
    return *a == '\0' && *b == '\0';
}

// Carga todos los registros del archivo CSV (por defecto 'ranking.csv') a un array.
// Devuelve el número de registros leídos, o -1 si hay error.
int loadRanking(PlayerRecord arr[], int max, const char *filePath) {
    FILE *f = fopen(filePath ? filePath : RANKING_FILE, "r");
    if (!f) return -1;
    int n = 0;
    while (n < max && !feof(f)) {
        PlayerRecord pr;
        if (fscanf(f, "%31[^,],%d,%d,%d,%d\n",
                   pr.name, &pr.wins, &pr.draws, &pr.losses, &pr.score) == 5) {
            arr[n++] = pr;
        } else {
            // Si una linea no contiene valores o hay error (,), se saltea.
            int ch; while ((ch = fgetc(f)) != '\n' && ch != EOF) {}
        }
    }
    fclose(f);
    return n;
}

// Sobrescribe el archivo CSV (modo 'w') con todos los registros del array.
int saveAllRanking(const PlayerRecord arr[], int n, const char *filePath) {
    FILE *f = fopen(filePath ? filePath : RANKING_FILE, "w");
    if (!f) return 0;
    for (int i = 0; i < n; ++i) {
        if (fprintf(f, "%s,%d,%d,%d,%d\n",
                    arr[i].name, arr[i].wins, arr[i].draws,
                    arr[i].losses, arr[i].score) <= 0) {
            fclose(f);
            return 0; // Error de escritura
        }
    }
    fclose(f);
    return 1;
}

// Actualiza (acumulando) o Inserta un resultado. Carga todo, modifica y guarda.
int upsertResult(const char *name, int wins, int draws, int losses, int score) {
    
    if (!name || !*name) return 0;

    PlayerRecord recs[MAX_RANKING_ENTRIES];
    int n = loadRanking(recs, MAX_RANKING_ENTRIES, RANKING_FILE);
    if (n < 0) n = 0; /* archivo no existe aun, empezamos en 0 */

    int idx = -1;
    // Buscar si el jugador ya existe (ignorando mayúsculas)
    for (int i = 0; i < n; ++i) {
        if (ieq(recs[i].name, name)) { idx = i; break; }
    }
    if (idx < 0) { /* nuevo jugador */
        if (n >= MAX_RANKING_ENTRIES) return 0; // Array lleno
        strncpy(recs[n].name, name, NAME_MAX - 1);
        recs[n].name[NAME_MAX - 1] = '\0';
        recs[n].wins   = wins;
        recs[n].draws  = draws;
        recs[n].losses = losses;
        recs[n].score  = score;
        n++;
    } else { /* actualizar acumulando stats */
        recs[idx].wins   += wins;
        recs[idx].draws  += draws;
        recs[idx].losses += losses;
        recs[idx].score  += score;
    }
    // Guardar todos los registros de vuelta al archivo
    return saveAllRanking(recs, n, RANKING_FILE);
}

// Función comparadora para qsort
static int comparePlayers(const void *a, const void *b) {
    const PlayerRecord *pA = (const PlayerRecord *)a;
    const PlayerRecord *pB = (const PlayerRecord *)b;

    // 1. Mayor puntaje primero (Descendente)
    if (pB->score != pA->score) return pB->score - pA->score;
    
    // 2. Desempate por nombre alfabético (Ascendente)
    return strcmp(pA->name, pB->name);
}

// Ordena el array de registros por puntaje (descendente).
// Usa el nombre (ascendente) como criterio de desempate.
void sortRankingDesc(PlayerRecord arr[], int n) {
    qsort(arr, n, sizeof(PlayerRecord), comparePlayers);
}

// Muestra el ranking formateado en la consola.
void showRanking(const PlayerRecord arr[], int n) {
    
    // Puntero para guardar el registro de la PC si lo encontramos
    const PlayerRecord* pcRecord = NULL; 
    
    // Contador separado para el ranking de jugadores (para que no se salte números)
    int rankCounter = 1; 

    // --- 1. Imprimir la tabla de Jugadores Humanos ---
    puts("+----+------------------------------+---+---+---+--------+");
    puts("| #  | Jugadores                    | G | E | P | Puntaje|");
    puts("+----+------------------------------+---+---+---+--------+");
    
    for (int i = 0; i < n; ++i) {
        
        // Comparamos el nombre. strcmp() devuelve 0 si son idénticos.
        if (strcmp(arr[i].name, AI_PLAYER_NAME) == 0) {
            // Si encontramos a la PC, guardamos sus datos
            pcRecord = &arr[i];
            // Y nos saltamos la impresión en esta tabla
            continue; 
        }
        
        // Si no es la PC, es un jugador. Lo imprimimos.
        printf("| %-2d | %-28s | %-1d | %-1d | %-1d | %-6d |\n",
            rankCounter, // Usamos el contador de ranking separado
            arr[i].name, arr[i].wins, arr[i].draws, arr[i].losses, arr[i].score);
        
        // Incrementamos el ranking solo si imprimimos un jugador
        rankCounter++;
    }
    
    puts("+----+------------------------------+---+---+---+--------+");

    
    // --- 2. Imprimir la tabla de la IA (si se encontró) ---
    if (pcRecord != NULL) {
        puts("\n+--------------------------------------------------------+");
        puts("| Estadisticas de la IA (PC)                             |");
        puts("+----+------------------------------+---+---+---+--------+");
        // Nota: No usamos 'rankCounter' aquí, solo mostramos sus stats
        printf("     | %-28s | %-1d | %-1d | %-1d | %-6d |\n",
               pcRecord->name, pcRecord->wins, pcRecord->draws, 
               pcRecord->losses, pcRecord->score);
        puts("+----+------------------------------+---+---+---+--------+");
    }
}