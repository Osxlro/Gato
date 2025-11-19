/*
 * Nombre del archivo: io.h
 *
 * Responsabilidades:
 * - Definir la estructura de datos 'PlayerRecord' que representa
 * una entrada en el ranking.
 * - Declarar (prototipar) todas las funciones relacionadas con la
 * lectura, escritura y manipulación del archivo de ranking ('ranking.csv').
 * - Definir constantes como 'NAME_MAX' para unificar el tamaño
 * de los nombres en todo el proyecto.
 */

#ifndef IO_H
#define IO_H

// Define la longitud máxima permitida para el nombre de un jugador.
#define NAME_MAX 32

/*
 * Estructura para almacenar los datos de un jugador en el ranking.
 * Contiene el nombre y sus estadísticas de juego (victorias, empates,
 * derrotas) y el puntaje total calculado.
 */
typedef struct {
    char name[NAME_MAX];
    int  wins, draws, losses;
    int  score;
} PlayerRecord;

// Carga el contenido del archivo CSV (ranking) en un array de 'PlayerRecord'.
// Devuelve el número de registros leídos (n) o -1 si hay error.
int loadRanking(PlayerRecord arr[], int max);

// Ordena un array de 'PlayerRecord' por puntaje (descendente)
// y luego por nombre (ascendente) como desempate.
void sortRankingDesc(PlayerRecord arr[], int n);

// Imprime en la consola el ranking (un array de 'PlayerRecord')
// con un formato de tabla.
void showRanking(const PlayerRecord arr[], int n);

// Función principal para guardar un resultado: "Update" (actualizar) o
// "Insert" (insertar). Carga el ranking, busca al jugador,
// actualiza sus estadísticas (si existe) o lo añade (si es nuevo),
// y luego guarda todo el ranking de nuevo en el archivo.
int  upsertResult(const char *name, int wins, int draws, int losses, int score);

#endif