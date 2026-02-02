/*
 * src/config.h
 *
 * Responsabilidades:
 * - Centralizar todas las constantes de configuración del proyecto.
 * - Evitar el "hardcoding" (valores mágicos) en el código fuente.
 */

#ifndef CONFIG_H
#define CONFIG_H

// --- Versiones ---
#define GAME_VERSION        "1.0.0"
#define PROTOCOL_VERSION    1       // Version del protocolo de red

// --- Red (Network) ---
#define GAME_PORT           8888    // Puerto TCP principal
#define DISCOVERY_PORT      8889    // Puerto UDP para broadcast
#define DISCOVERY_TIMEOUT   30      // Segundos para buscar partida
#define HOST_TIMEOUT        60      // Segundos que el host espera conexión

#define TIMEOUT_SEC         10      // Timeout de desconexión en partida (TCP)
#define HB_INTERVAL         2       // Intervalo de Heartbeat (segundos)
#define HB_MSG              0xFF    // Byte de Heartbeat

#define DISCOVERY_MSG       "GATO_REQ"
#define DISCOVERY_ACK       "GATO_ACK"

// --- Datos y Persistencia (IO) ---
#define NAME_MAX            32      // Longitud máxima de nombre de jugador
#define MAX_RANKING_ENTRIES 2000    // Límite de registros en memoria para el ranking
#define RANKING_FILE        "ranking.csv"

#endif
