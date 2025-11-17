# GATO JUEGO PROYECTO

> Nombre: Oscurin   |   Programación Estructurada  |  Fecha: 2025-11-16


## PARTE I - MAPA DE NAVEGACIÓN

> Una vez haya terminado de observar la Sección Parte I, me gustaría tu feedback: https://forms.gle/eRgVNe2f9kKL2PhF7 **Sería muy útil si me apoya con eso**

### Mapa de navegación (alto nivel)

```
[Inicio]
  ├─ 1) Jugar
  │     ├─ a) Jugador vs Jugador
  │     │      └─ Tablero 3x3 → Resultado → ¿Jugar de nuevo?
  │     └─ b) Jugador vs PC
  │            └─ Tablero 3x3 → Resultado → ¿Jugar de nuevo?
  ├─ 2) Tabla de posiciones (ranking)
  ├─ 3) Ayuda / Instrucciones
  └─ 4) Salir
```

---

### Pantalla: Inicio

```
+--------------------------------------------------+
|                   (Gato)                         |
+--------------------------------------------------+
| 1) Jugar                                         |
| 2) Tabla de posiciones                           |
| 3) Ayuda / Instrucciones                         |
| 4) Salir                                         |
+--------------------------------------------------+
Seleccione una opción: _
```

### Pantalla: Selección de modo de juego
```
[1] Seleccione modo:
a) Jugador vs Jugador
b) Jugador vs PC
c) Volver
Opcion: _
```

### Pantalla: Captura de nombres
Se muestra al inicio de ``playPVP`` o ``playPVC``
```
Nombre Jugador 1: ____________
Nombre Jugador 2: ____________ // Solo aplica en playPVP
```

### Pantalla: Tablero 3x3
```
    Col: 1   2   3
Fil       |   |   
 1        |   |
       ---+---+---
 2        |   |
       ---+---+---
 3        |   |

Turno: <Nombre> (<X/O>)
Ingrese fila y columna ([1 3]..[2 1]..): _
```

#### Mensajes de validación (ejemplos)
- **Entrada inválida**: si no son números.
- **Fuera de rango**: si no es 1, 2 o 3.
- **Casilla ocupada**: si la celda ya tiene 'X' o 'O'.
- **Turno**: alterna automáticamente.
- **Victoria**: muestra línea ganadora (h, v, d) y nombre.
- **Empate**: tablero lleno sin ganador.

### Pantalla: Resultado
```
Resultado: [Gana <Nombre> / Empate]
¿Jugar de nuevo? (s/n): _
```

### Pantalla: Tabla de posiciones (Ranking)
```
+----+------------------------------+---+---+---+--------+
| #  | Jugadores                    | G | E | P | Puntaje|
+----+------------------------------+---+---+---+--------+
| 1  | Humano1                      | 2 | 1 | 0 | 7      |
| 2  | Humano2                      | 0 | 1 | 2 | 1      |
+----+------------------------------+---+---+---+--------+

+--------------------------------------------------------+
| Estadisticas de la IA (PC)                             |
+------------------------------+---+---+---+-------------+
| Jarvis (PC)                  | 2 | 0 | 0 | 6           |
+------------------------------+---+---+---+-------------+

Presiona Enter para continuar...
```

### Pantalla: Ayuda / Instrucciones
```
[AYUDA]
- **Objetivo**: Alinear 3 simbolos iguales (X u O).
- **Entrada**:  Coordenadas fila columna ('1 3'..'1 4'..).
- **Puntaje sugerido**:
  G=3, E=1, P=0.

Se guarda el Score del usuario en el Ranking.

Presiona Enter para continuar...
```
---
#### Notas de accesibilidad (terminal)
- Contraste alto y etiquetas claras.
- Atajos de teclado simples (números/letras).
- Mensajes de error concretos.

---
# PARTE II - DIAGRAMA DE BLOQUES

```
[INICIO]
   |
   v
[MENU PRINCIPAL] (mainMenuLoop)
   |--(1) Jugar -----------------------------.
   |                                         |
   |                                         v
   |                            [Seleccionar modo] (a/b/c)
   |                               |        |
   |                               |        +--(b) playPVC()
   |                         _     |                    |
   |                               v                    v
   |                      (a) playPVP()        [askHumanName]
   |                               |                    |
   |                               v                    v
   |                          [askPlayerNames]   [Init tablero]
   |                               |                    |
   |                               v                    v
   |                          [Init tablero]      [Bucle de turnos]
   |                               |                    |
   |                               v                    v
   |                          [Bucle de turnos] --> [Leer/validar jugada]
   |       _                       |               (o) [pcMove]
   |                               v                    |
   |                         [Check win/empate] <--------
   |             ------------------+------------------.
   |             |                                     |
   |             v                                     v
   |       [Victoria]                           [Empate]
   |             \___________________________________/
   |                                          v
   |                                  [Calcular puntaje]
   |     _                                    |
   |                                         v
   |                               [Guardar en (upsertResult)]
   |                                         |
   |                                         v
   |                                  [¿Jugar de nuevo? (s/n)]
   |                                     |        |
   |<-(s)--------------------------------'        | (n)
   |                                              v
   '------------------------------------------[VOLVER al Menú]
   |
   +--(2) Tabla de posiciones --> [loadRanking] -> [sortRankingDesc] -> [showRanking] -> [VOLVER]
   |
   +--(3) Ayuda/Instrucciones  --> [showHelp]   --> [VOLVER]
   |
   +--(4) Salir  ----------------------------------------> [FIN]
```
---
# PARTE III — Diseño técnico (C)

> **Suposiciones de entorno**  
> - Compilador: gcc (C11/C17).  
> - Arquitectura x64 típica: `char`=1 B, `int`=4 B, `double`=8 B, puntero=8 B.  
> - Interfaz: consola/terminal.  
> - Paradigma: **estructurado** y **modular** (multiarchivo).  
> - **Sin variables globales**: datos se pasan por parámetros (valor/referencia).

---

## 1) Tipos de datos (justificación y estimación de bytes)

### 1.1 Tablero de juego
```c
char board[3][3];  // 'X', 'O' o ' ' (vacío)
```
- **Por qué `char`**: mínimo almacenamiento y se imprime fácil en consola.  
- **Memoria**: `3 * 3 * 1 = 9 bytes`.

---

### 1.2 Nombres de jugadores
```c
#define NAME_MAX 32
char nameP1[NAME_MAX];
char nameP2[NAME_MAX];   // En JvPC será "PC"
```
- **Razón**: 32 chars alcanzan para nombres comunes y evitan overflow.  
- **Memoria**: `2 * 32 = 64 bytes`.

---

### 1.3 Contadores y puntaje por partida
```c
int winsP1, drawsP1, lossesP1;
int winsP2, drawsP2, lossesP2;
int scoreP1, scoreP2;    // Puntaje = 3*G + 1*E + 0*P
```
- **Uso**: métricas al terminar cada partida para persistencia.  
- **Memoria**: `8 * 4 = 32 bytes`.

> Nota: podría reducirse a contadores solo de la **partida actual** y calcular el puntaje al final.

---

### 1.4 Control de flujo de juego / entrada
```c
int currentPlayer;  // 0 o 1
int row, col;       // 1..3
int movesCount;     // 0..9
int option;         // menú
```
- **Memoria estimada**: `~16–20 bytes`.

---

### 1.5 Estructura para **ranking** (persistencia y consulta)
```c
typedef struct {
    char name[NAME_MAX];
    int  wins, draws, losses;
    int  score;
} PlayerRecord;

#define MAX_RECORDS 1000
PlayerRecord ranking[MAX_RECORDS];
int rankingCount;
```
- **Por qué `struct`**: agrupa datos del jugador y facilita ordenamiento.  
- **Memoria por registro**: `32 + 4*4 = 48 bytes`.  
- **Para 1000 registros**: `~48 KB`. Ajusta `MAX_RECORDS` a tus necesidades.

---

### 1.6 Estimación total (runtime sin ranking cargado)
- Tablero: **9 B**  
- Nombres: **64 B**  
- Contadores/flujo: **< 64 B**  
- **Total fijo**: **≤ 150 B** (+ stack y buffers de E/S).  
- Al cargar ranking completo (1000): **~50 KB** adicionales.

---

## 2) Diseño de funciones (qué hacen, validaciones, parámetros, retorno)

> **Convenciones**  
> - **Por valor**: escalares (`int`, `char`) cuando no se requiere modificar el argumento del llamador.  
> - **Por referencia**: arreglos/structs o cuando el callee **modifica** el dato del llamador.  
> - **`const`** para garantizar no modificación de datos referenciados.

### 2.1 Módulo **UI** (`ui.h` / `ui.c`)
- `void clearScreen(void);`  
  **Hace**: limpia consola (método simple y portable).  
  **Parámetros/Retorno**: n/a.

- `void showMainMenu(void);`  
  **Hace**: imprime menú (Jugar, Ranking, Ayuda, Salir).

- `void printBoard(const char board[3][3]);`  
  **Hace**: dibuja el tablero.  
  **Parámetros**: `board` por referencia constante.

- `void showHelp(void);`  
  **Hace**: muestra instrucciones.

- `void pauseEnter(void);`  
  **Hace**: espera `Enter` para continuar.

---

### 2.2 Módulo **Game** (`game.h` / `game.c`)
- `void initBoard(char board[3][3]);`  
  **Hace**: rellena con `' '`.

- `int isValidCell(int r, int c);`  
  **Valida**: `1..3` en ambas coordenadas.  
  **Retorno**: `1` válido / `0` inválido.

- `int isCellEmpty(const char board[3][3], int r, int c);`  
  **Valida**: casilla libre (`' '`).  
  **Retorno**: `1` libre / `0` ocupada.

- `int applyMove(char board[3][3], int r, int c, char sym);`  
  **Hace**: coloca `sym` si la celda es válida y libre.  
  **Retorno**: `1` éxito / `0` fallo.

- `int checkWin(const char board[3][3], char sym);`  
  **Hace**: evalúa 8 líneas (3 filas, 3 columnas, 2 diagonales).  
  **Retorno**: `1` gana / `0` no.

- `int boardFull(const char board[3][3]);`  
  **Retorno**: `1` si no hay `' '`.

- `int readMove(int *r, int *c);`  
  **Hace**: lee `r` y `c` de `stdin` (maneja errores de formato / `q` opcional).  
  **Parámetros**: por referencia.  
  **Retorno**: `1` ok / `0` cancelado o inválido persistente.

- `int scoreOf(int wins, int draws, int losses);`  
  **Hace**: `3*wins + 1*draws + 0*losses`.

- `void playPVP(char name1[], char name2[]);`  
  **Hace**: ciclo de turnos humano vs humano.

- `void playPVC(char name1[]);`  
  **Hace**: humano vs PC (PC es "PC").

---

### 2.3 Módulo **AI** (`ai.h` / `ai.c`)
- `void pcMove(char board[3][3], char pcSym, char humanSym);`  
  **Heurística** (en orden):  
  1) Ganar si hay jugada inmediata.  
  2) Bloquear victoria del humano.  
  3) Tomar centro.  
  4) Tomar esquina libre.  
  5) Tomar lateral libre.  
  **Parámetros**: tablero por referencia; símbolos por valor.

---

### 2.4 Módulo **IO / Persistencia** (`io.h` / `io.c`)
> Archivo: `ranking.csv` con columnas `Nombre,G,E,P,Puntaje`.

- `int saveResult(const char *name, int wins, int draws, int losses, int score);`  
  **Hace**: `append` de una fila al CSV.  
  **Valida**: `name` no vacío, `fopen` no `NULL`.  
  **Retorno**: `1` ok / `0` error.

- `int loadRanking(PlayerRecord arr[], int max, const char *filePath);`  
  **Hace**: lee hasta `max` registros; ignora líneas corruptas.  
  **Retorno**: **n** (≥0) o `-1` si error de archivo.

- `void sortRankingDesc(PlayerRecord arr[], int n);`  
  **Hace**: ordena por `score` descendente (y `name` ascendente como desempate opcional).

- `void showRanking(const PlayerRecord arr[], int n);`  
  **Hace**: imprime tabla formateada en consola.

---

### 2.5 Módulo **App / Menú** (`app.h` / `app.c`)
- `int mainMenuLoop(void);`  
  **Hace**: ciclo de **Menú Principal** (Jugar, Ranking, Ayuda, Salir).  
  **Retorno**: `0` al salir.

- `void startGameFlow(void);`  
  **Hace**: pide **modo** (JvJ/JvPC), captura nombres, ejecuta partida y al final guarda resultado y ofrece “¿Jugar de nuevo?”.

---

## 3) Prototipos ejemplo (headers)

### `game.h`
```c
#ifndef GAME_H
#define GAME_H

// Nombre de la IA
#define AI_PLAYER_NAME "Jarvis (PC)"

void initBoard(char board[3][3]);
int ADC isValidCell(int r, int c);
int  isCellEmpty(const char board[3][3], int r, int c);
int  applyMove(char board[3][3], int r, int c, char sym);
int  checkWin(const char board[3][3], char sym);
int  boardFull(const char board[3][3]);
int  readMove(int *r, int *c);
int  scoreOf(int wins, int draws, int losses);
void playPVP(void);
void playPVC(void)

#endif
```

### `ai.h`
```c
#ifndef AI_H
#define AI_H
void pcMove(char board[3][3], char pcSym, char humanSym);
#endif
```

### `io.h`
```c
#ifndef IO_H
#define IO_H

#define NAME_MAX 32

typedef struct {
    char name[NAME_MAX];
    int  wins, draws, losses;
    int  score;
} PlayerRecord;

int  saveResult(const char *name, int wins, int draws, int losses, int score);
int  loadRanking(PlayerRecord arr[], int max, const char *filePath);
void sortRankingDesc(PlayerRecord arr[], int n);
void showRanking(const PlayerRecord arr[], int n);

#endif
```

### `ui.h`
```c
#ifndef UI_H
#define UI_H
void clearScreen(void);
void showMainMenu(void);
void printBoard(const char board[3][3]);
void showHelp(void);
void pauseEnter(void);
#endif
```

---

## 4) Validaciones clave (resumen)
- **Entrada de jugada**: `r` y `c` en `1..3`; si no, mensaje de error y reintentar.  
- **Casilla ocupada**: rechazar y volver a pedir.  
- **Victoria**: evaluar tras cada jugada con `checkWin`.  
- **Empate**: `boardFull(...)` y nadie ganó.  
- **Puntuación**: `score = 3*wins + 1*draws`.  
- **Persistencia**: guardar con `saveResult(...)` siempre al terminar.  
- **Ranking**: `loadRanking` → `sortRankingDesc` → `showRanking`.  
- **Rejugar**: preguntar “¿Jugar de nuevo? (s/n)” y validar.

---

## 5) Estimación de memoria (ejemplo)
- **Durante partida** (sin ranking cargado):  
  - Tablero: 9 B  
  - Nombres: 64 B  
  - Contadores/flujo: < 64 B  
  - **Total** ≈ **≤ 150 B** (más stack temporal).  
- **Ranking cargado (1000)**:  
  - `ranking[]` ≈ **48 KB**  
---
