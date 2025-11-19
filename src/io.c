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
#include "cJSON.h"  // Para manejo de JSON (Firebase)

// Librerias del Lenguaje
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <curl.h>

// Guardar RESPONSE libcurl
struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    printf("No hay memoria suficiente. (realloc falló)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

// Carga todos los registros del archivo CSV (por defecto 'ranking.csv') a un array.
// Devuelve el número de registros leídos, o -1 si hay error.
int loadRanking(PlayerRecord arr[], int max) {
CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);  // Empezará en 1 byte y crecerá
    chunk.size = 0;

    const char *firebase_url = "https://gato-database-default-rtdb.europe-west1.firebasedatabase.app/ranking.json";

    curl = curl_easy_init();
    if(!curl) {
        fprintf(stderr, "curl_easy_init() falló\n");
        free(chunk.memory);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, firebase_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); // Callback para guardar
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk); // Puntero a nuestra memoria
    
    // (Recuerda añadir el hack de SSL por ahora)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    res = curl_easy_perform(curl);

    int playerCount = 0;
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() falló: %s\n", curl_easy_strerror(res));
    } else {
        // ¡ÉXITO! Ahora parseamos el JSON que está en chunk.memory
        cJSON *json = cJSON_Parse(chunk.memory);
        
        if (json == NULL) {
            fprintf(stderr, "Error parseando JSON.\n");
        } else {
            // El JSON es un objeto grande (ranking). 
            // Iteramos sobre cada "hijo" (cada jugador).
            cJSON *player_json = NULL;
            cJSON_ArrayForEach(player_json, json)
            {
                if (playerCount >= max) break; // No sobrepasar el array

                // Extraemos los datos de cada jugador
                cJSON *name = cJSON_GetObjectItem(player_json, "name");
                cJSON *wins = cJSON_GetObjectItem(player_json, "wins");
                cJSON *draws = cJSON_GetObjectItem(player_json, "draws");
                cJSON *losses = cJSON_GetObjectItem(player_json, "losses");
                cJSON *score = cJSON_GetObjectItem(player_json, "score");

                // Copiamos los datos al array 'arr'
                if (cJSON_IsString(name)) strncpy(arr[playerCount].name, name->valuestring, NAME_MAX);
                if (cJSON_IsNumber(wins)) arr[playerCount].wins = wins->valueint;
                if (cJSON_IsNumber(draws)) arr[playerCount].draws = draws->valueint;
                if (cJSON_IsNumber(losses)) arr[playerCount].losses = losses->valueint;
                if (cJSON_IsNumber(score)) arr[playerCount].score = score->valueint;
                
                playerCount++;
            }
            cJSON_Delete(json); // Liberar memoria de cJSON
        }
    }

    curl_easy_cleanup(curl);
    free(chunk.memory);
    
    return playerCount; // Devuelve el número de jugadores encontrados
}

// Actualiza (acumulando) o Inserta un resultado. Carga todo, modifica y guarda.
int upsertResult(const char *name, int wins, int draws, int losses, int score) {
char firebase_url[256];
    char json_data[256];
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk; // Para guardar la respuesta del GET

    chunk.memory = malloc(1);
    chunk.size = 0;
    
    // --- 1. CONSTRUIR URL DEL JUGADOR ---
    // Ej: https://.../ranking/Jarvis (PC).json
    sprintf(firebase_url, 
            "https://gato-database-default-rtdb.europe-west1.firebasedatabase.app/%s.json", 
            name);

    // --- 2. LEER (GET) DATOS ANTIGUOS ---
    curl = curl_easy_init();
    if(!curl) { /* ... manejo de error ... */ return 0; }
    
    curl_easy_setopt(curl, CURLOPT_URL, firebase_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Hack SSL
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl); // Limpiamos la sesión GET

    PlayerRecord oldData = {0}; // Inicializar a ceros
    if(res == CURLE_OK) {
        // Parsear la respuesta
        cJSON *json = cJSON_Parse(chunk.memory);
        // Si json no es NULL, Firebase devolvió un objeto de jugador
        if (json != NULL) { 
            cJSON *j_wins = cJSON_GetObjectItem(json, "wins");
            cJSON *j_draws = cJSON_GetObjectItem(json, "draws");
            cJSON *j_losses = cJSON_GetObjectItem(json, "losses");
            cJSON *j_score = cJSON_GetObjectItem(json, "score");

            if (cJSON_IsNumber(j_wins)) oldData.wins = j_wins->valueint;
            if (cJSON_IsNumber(j_draws)) oldData.draws = j_draws->valueint;
            if (cJSON_IsNumber(j_losses)) oldData.losses = j_losses->valueint;
            if (cJSON_IsNumber(j_score)) oldData.score = j_score->valueint;
            
            cJSON_Delete(json);
        }
        // Si json es NULL, significa que el jugador no existía (era "null")
        // y oldData se quedará en ceros, lo cual es correcto.
    }
    free(chunk.memory); // Liberar memoria del GET

    // --- 3. SUMAR LOS DATOS NUEVOS ---
    int total_wins = oldData.wins + wins;
    int total_draws = oldData.draws + draws;
    int total_losses = oldData.losses + losses;
    int total_score = oldData.score + score;

    // --- 4. ESCRIBIR (PUT) DATOS TOTALES ---
    
    // Construir el JSON con los totales
    sprintf(json_data, 
            "{\"name\": \"%s\", \"wins\": %d, \"draws\": %d, \"losses\": %d, \"score\": %d}", 
            name, total_wins, total_draws, total_losses, total_score);

    curl = curl_easy_init(); // Nueva sesión de curl para PUT
    if(!curl) { return 0; }

    curl_easy_setopt(curl, CURLOPT_URL, firebase_url); // Misma URL
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT"); 
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data); // JSON con totales
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Hack SSL
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() (PUT) falló: %s\n", curl_easy_strerror(res));
        return 0; // Fallo
    }
    
    return 1; // ¡Éxito!
}

// Ordena el array de registros por puntaje (descendente).
// Usa el nombre (ascendente) como criterio de desempate.
void sortRankingDesc(PlayerRecord arr[], int n) {
    // Ordenamiento de burbuja simple
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