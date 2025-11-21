# GATO JUEGO PROYECTO

> **Nombre:** Oscurin | **Programación Estructurada** | **Fecha:** 2025-11-16 | **Plataforma:** Windows

## PARTE I - MAPA DE NAVEGACIÓN
### Mapa de navegación (alto nivel)

```
[Inicio]
  ├─ 1) Jugar
  │     ├─ a) Jugador vs Jugador (Local)
  │     ├─ b) Jugador vs PC (IA)
  │     └─ c) Jugador vs Jugador Online (LAN)
  │            ├─ 1. Crear Partida (Host)
  │            ├─ 2. Buscar Partida (Auto-descubrimiento)
  │            └─ 3. Conectar Manual (IP)
  ├─ 2) Tabla de posiciones (Ranking Local)
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
c) Jugador vs Jugador Online
d) Volver
Opcion: _
```

### Pantalla Selección de Nombre
```
-- Modo (*)
Nombre Jugador 1: ___
Nombre Jugador 2: ___ (Solo aplica en PVP)
```

### Pantalla: Modo Online
```
Jugador: Oscurin
---------------------------
1. Crear Partida (Host)
2. Buscar Partida (Auto)
3. Conectar Manual (IP)
4. Volver
Opcion: _
```

### Pantalla: Tablero 3x3 (En Partida)
```
Modo: (*) | Turno: Jugador1 (X)
------------------------------

    Col: 1   2   3
Fil       |   |   
 1      X |   |
       ---+---+---
 2        | O |
       ---+---+---
 3        |   |

Ingresa fila y columna (o 'q' para salir): _
```
> **NOTA: '*'**: Sera el modo que corresponda (PVP, PVC o Online)

#### Mensajes de validación y control
- **Entrada inválida**: Limpia la pantalla y pide reingresar.
- **Fuera de rango / Ocupada**: Muestra error específico y reintenta.
- **Rendición ('q')**: Termina la partida, envía señal al rival y otorga la victoria.
- **Revancha (Online)**: Al terminar, ambos jugadores votan (s/n) para jugar de nuevo.

### Pantalla: Tabla de posiciones (Ranking)
```
+----+------------------------------+---+---+---+--------+
| #  | Jugadores                    | G | E | P | Puntaje|
+----+------------------------------+---+---+---+--------+
| 1  | Oscurin                      | 5 | 2 | 1 | 17     |
| 2  | Pepito                       | 1 | 2 | 5 | 5      |
+----+------------------------------+---+---+---+--------+

+--------------------------------------------------------+
| Estadisticas de la IA (PC)                             |
+------------------------------+---+---+---+-------------+
| Jarvis (PC)                  | 8 | 1 | 0 | 25          |
+------------------------------+---+---+---+-------------+
```

### Pantalla: Ayuda / Instrucciones
```
[AYUDA]
- **Objetivo**: Alinear 3 simbolos iguales (X u O).
- **Entrada**:  Coordenadas fila columna ('1 3'..'1 4'..).
- **Puntaje sugerido**:
  G=3, E=1, P=0.

Se guarda el Score del usuario en el Ranking local.
```

---
# PARTE II - DIAGRAMA DE BLOQUES

```
[INICIO]
   |
   v
[MENU PRINCIPAL] (mainMenuLoop)
   |--(1) Jugar -----------------------------.
   |                                         |
   |                                         v
   |                                [Seleccionar modo]
   |                                   |    |    |
   |                                   |    |    +--(c) playOnline()
   |                                   |    |           |
   |                                   |    |           v
   |                                   |    |     [Menu Conexión]
   |                                   |    |      |          |
   |                                   |    |    Host       Cliente
   |                                   |    |   (TCP/UDP)  (TCP/UDP)
   |                                   |    |      |          |
   |                                   |    |      v          v
   |                                   |    |     [Intercambiar Nombres]
   |                                   |    |           |
   |                                   |    +--(b) playPVC()
   |                                   |            |
   |                                   v            v
   |                             (a) playPVP()    [askHumanName]
   |                                   |            |
   |                                   v            v
   |                            [askPlayerNames]  [Init tablero]
   |                                   |            |
   |                                   v            v
   |                              [Init tablero]   [Bucle de turnos]
   |                                   |            |
   |                                   v            v
   |                           [Bucle de turnos] --> [getValidMove]
   |                                   |            |
   |                                   v            v
   |                          [Check win/empate] <---
   |                                   |
   |             .---------------------+---------------------.
   |             |                                           |
   |             v                                           v
   |        [Victoria]                                    [Empate]
   |             \___________________________________________/
   |                                     v
   |                            [Calcular puntaje]
   |                                     |
   |                                     v
   |                         [Guardar en (upsertResult)]
   |                                     |
   |                                     v
   |                           [¿Jugar de nuevo? (s/n)]
   |                                  |        |
   |<-(s)-----------------------------'        | (n)
   |                                           v
   '-----------------------------------[VOLVER al Menú]
   |
   +--(2) Tabla de posiciones --> [loadRanking] -> [sortRankingDesc] -> [showRanking] -> [VOLVER]
   |
   +--(3) Ayuda/Instrucciones  --> [showHelp]   --> [VOLVER]
   |
   +--(4) Salir  ----------------------------------------> [FIN]
```

---
# PARTE III — DISEÑO TÉCNICO (C)

> **Suposiciones de entorno**
> - **Sistema Operativo:** Windows (Requerido por librería `winsock2`).
> - **Compilador:** GCC (MinGW recomendado).
> - **Arquitectura:** x64.
> - **Paradigma:** Estructurado y modular.

## 1) Estructura y Datos

### 1.1 Protocolo de Red (Modo Online)
Comunicación binaria directa sobre Sockets TCP/UDP (`network.c`).
- **Autodescubrimiento:** UDP Broadcast al puerto 8889.
- **Juego:** Conexión TCP persistente al puerto 8888.
- **Datos:**
    - Movimiento: 2 bytes `[fila, columna]`.
    - Handshake (intercambio de nombres): 32 bytes `[Nombre]`.

### 1.2 Persistencia Local
- **Archivo:** `ranking.csv`.
- **Formato:** `Nombre,Victorias,Empates,Derrotas,Puntaje`.

## 2) Diseño de Módulos

| Módulo | Archivo | Responsabilidad |
| :--- | :--- | :--- |
| **Core** | `main.c` | Punto de entrada. |
| **Controlador** | `app.c` | Menú principal y navegación. |
| **Lógica** | `game.c` | Reglas de Gato, validación (DRY), control de turnos PvP/PvC/Online. |
| **IA** | `ai.c` | Algoritmo para la computadora (intenta ganar, bloquear o tomar centro). |
| **Red** | `network.c` | Uso del winsock (TCP, UDP, Socket). |
| **Datos** | `io.c` | Gestión de archivo CSV (Ranking). |
| **Vista** | `ui.c` | Dibujado en consola. |

## 3. Definición de Tipos de Datos
A continuación se describen las estructuras de datos principales, justificando su elección para optimizar el uso de memoria y rendimiento.

### 1.1 Tablero de Juego (`board`)
- **Tipo:** `char board[3][3]`
- **Argumentación:** Se utiliza `char` (1 byte) en lugar de `int` (4 bytes) porque cada celda solo necesita almacenar 3 estados posibles: `'X'`, `'O'`, o `' '` (vacío). Usar enteros desperdiciaría memoria.
- **Estimación de Memoria:**
    - `3 filas * 3 columnas * 1 byte =` **9 bytes**.

### 1.2 Registro de Jugador (`PlayerRecord`)
Estructura utilizada para la persistencia de datos en el Ranking.
```c
typedef struct {
    char name[32];
    int wins, draws, losses;
    int score;
} PlayerRecord;
```
**Argumentación**: Se agrupan los datos en un struct para mantener la coherencia lógica y facilitar el ordenamiento (sorting) y la escritura en disco como un solo bloque. name se limita a 32 bytes para prevenir desbordamientos de búfer y mantener un tamaño fijo predecible.

**Estimación de Memoria** (por registro): ```name```: 32 bytes. ```int``` (x4): 4 bytes * 4 = 16 bytes.

Total: `48 bytes` por jugador. 
Nota: Para un ranking de 1000 jugadores, el consumo es por ~48 KB de RAM.

### 1.3 Protocolo de Red (Buffers)
Tipo: `char buffer[2]` (para movimientos) y `char buffer[32]` (para nombres/IPs).

**Argumentación**: La comunicación por Sockets TCP/UDP es costosa. Enviamos la mínima cantidad de datos posible (binarios) en lugar de textos largos (JSON/XML).

Estimación:
- **Paquete de movimiento**: 2 bytes (Fila + Columna).
- **Handshake**: 32 bytes fijos.

## 2. Definición de Funciones Principales
A continuación se detallan las funciones core del sistema, especificando su interfaz y comportamiento.

### Módulo Lógica (`game.c`)

#### `getValidMove`
- **Descripción:** Solicita al usuario una coordenada, verifica que el formato sea correcto y gestiona los reintentos si hay errores, limpiando la pantalla para mantener la UI ordenada.
- **Validaciones:**
  - Entrada numérica correcta (`sscanf`).
  - Rango de coordenadas (1 a 3).
  - Disponibilidad de la celda (que no esté ocupada).
- **Parámetros:**
  - `board` (Referencia - `const char[][]`): El estado actual del tablero para lectura.
  - `r`, `c` (Referencia - `int*`): Punteros donde se almacenarán la fila y columna válidas.
  - `header` (Referencia - `const char*`): Texto del título para redibujar la UI en caso de error.
- **Retorno:** `int` (1 = Éxito, -1 = Usuario se rinde).

#### `checkAndPrintEnd`
- **Descripción:** Evalúa el estado del tablero después de cada movimiento para determinar si la partida ha concluido por victoria o empate, y muestra el mensaje correspondiente.
- **Validaciones:** Verifica líneas horizontales, verticales y diagonales. Verifica si el tablero está lleno.
- **Parámetros:**
  - `board` (Referencia - `const char[][]`): Tablero a evaluar.
  - `sym` (Valor - `char`): Símbolo del jugador actual ('X' o 'O').
  - `winnerName` (Referencia - `const char*`): Nombre para mostrar en caso de victoria.
- **Retorno:** `int` (1 = Juego terminado, 0 = Juego continúa).

---

### Módulo Red (`network.c`)

#### `net_host_wait_for_client`
- **Descripción:** Inicia un servidor TCP y simultáneamente escucha peticiones UDP (Broadcast) para el autodescubrimiento, permitiendo que el Host sea encontrado sin escribir IPs.
- **Validaciones:** Comprueba que los puertos (8888 y 8889) estén libres y que `winsock` se haya iniciado.
- **Parámetros:**
  - `tcp_port` (Valor - `int`): El puerto donde se establecerá el juego.
- **Retorno:** `int` (ID del Socket del cliente conectado o -1 si hay error/timeout).

#### `net_send_move`
- **Descripción:** Empaqueta las coordenadas de la jugada en un buffer de 2 bytes y lo envía a través del socket TCP conectado.
- **Validaciones:** Verifica que la función `send` del sistema operativo retorne el número correcto de bytes enviados.
- **Parámetros:**
  - `socket` (Valor - `int`): Descriptor del socket activo.
  - `r`, `c` (Valor - `int`): Coordenadas de la jugada.
- **Retorno:** `int` (1 = Envío exitoso, 0 = Error de conexión).

---

### Módulo Persistencia (`io.c`)

#### `upsertResult`
- **Descripción:** Busca a un jugador en el archivo CSV. Si existe, suma sus estadísticas; si no, crea un nuevo registro al final.
- **Validaciones:** Verifica que el archivo `ranking.csv` pueda abrirse en modo lectura/escritura. Valida que el nombre no sea nulo.
- **Parámetros:**
  - `name` (Referencia - `const char*`): Nombre del jugador.
  - `wins`, `draws`, `losses`, `score` (Valor - `int`): Estadísticas a sumar.
- **Retorno:** `int` (1 = Guardado exitoso, 0 = Error de E/S).