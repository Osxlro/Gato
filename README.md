# GATO JUEGO PROYECTO

> Nombre: Oscurin   |   Programación Estructurada  |  Fecha: 2025-11-16


## PARTE I - MAPA DE NAVEGACIÓN

> Una vez haya terminado de observar la Sección Parte I, me gustaría tu feedback: https://forms.gle/eRgVNe2f9kKL2PhF7

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
|                 TIC-TAC-TOE (Gato)               |
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
+-------------------- MODO DE JUEGO ----------------+
| a) Jugador vs Jugador                             |
| b) Jugador vs PC                                  |
| c) Volver                                         |
+---------------------------------------------------+
Opción: _
```

### Pantalla: Captura de nombres
```
+-------------------- NOMBRES -----------------------+
| Ingrese nombre del Jugador 1 (X): ____________     |
| Ingrese nombre del Jugador 2 (O) / PC: ________    |
| (Dejar vacío para 'PC' si aplica)                  |
+----------------------------------------------------+
```

### Pantalla: Tablero 3x3
```
    Col: 1   2   3
Fil       |   |   
 1     1,1|1,2|1,3
       ---+---+---
 2     2,1|2,2|2,3
       ---+---+---
 3     3,1|3,2|3,3

Turno: <Nombre> (<X/O>)
Ingrese fila y columna (p.ej., 2 3) o 'q' para salir: _
```

#### Mensajes de validación (ejemplos)
- **Movimiento inválido**: casilla ocupada o fuera de rango (1-3).
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
+------------------ RANKING (desc) ------------------+
| # | Nombre        | G | E | P | Puntaje           |
|---+---------------+---+---+---+-------------------|
| 1 |               |   |   |   |                   |
| 2 |               |   |   |   |                   |
| 3 |               |   |   |   |                   |
+---------------------------------------------------+
(Ordenado por Puntaje desc). Enter para volver.
```

### Pantalla: Ayuda / Instrucciones
- Objetivo: alinear 3 símbolos iguales (X u O) en línea.
- Entrada: coordenadas **fila columna** (1..3 1..3).
- Puntuación sugerida: **G=3, E=1, P=0** (configurable).
- Persistencia: se guarda archivo `ranking.csv` al final de cada partida.

---
#### Notas de accesibilidad (terminal)
- Contraste alto y etiquetas claras.
- Atajos de teclado simples (números/letras).
- Mensajes de error concretos.

## PARTE I - MAPA DE NAVEGACIÓN

```
[INICIO]
   |
   v
[MENU PRINCIPAL]
   |--(1) Jugar -----------------------------.
   |                                         |
   |                                         v
   |                            [Seleccionar modo]
   |                               |        |
   |                               |        +--(b) Jugador vs PC
   |                               |                    |
   |                               v                    v
   |                      (a) Jugador vs Jugador   [Captura nombres]
   |                               |                    |
   |                               v                    v
   |                          [Captura nombres]   [Init tablero]
   |                               |                    |
   |                               v                    v
   |                          [Init tablero]      [Bucle de turnos]
   |                               |                    |
   |                               v                    v
   |                          [Bucle de turnos] --> [Leer/validar jugada]
   |                               |                    |
   |                               v                    v
   |                         [Check win/empate] <--------
   |                               |
   |             .------------------+------------------.
   |             |                                     |
   |             v                                     v
   |       [Victoria]                           [Empate]
   |             \___________________________ ________/
   |                                         V
   |                                  [Calcular puntaje]
   |                                         |
   |                                         v
   |                               [Guardar en archivo]
   |                                         |
   |                                         v
   |                                  [Mostrar ranking]
   |                                         |
   |                                         v
   |                               [¿Jugar de nuevo?]
   |                                   |        |
   |                                   |s/n     | n
   |                                   v        v
   |                            [Reiniciar]  [VOLVER]
   |
   +--(2) Tabla de posiciones --> [Mostrar ranking] --> [VOLVER]
   |
   +--(3) Ayuda/Instrucciones  --> [Mostrar ayuda]   --> [VOLVER]
   |
   +--(4) Salir  ----------------------------------------> [FIN]
```

Notas:
- Persistencia en CSV al terminar cada partida: nombre, G/E/P, puntaje.
- Ranking ordenado por puntaje (descendente).
- Validaciones: entrada (1..3), casilla libre, opción de menú correcta.
