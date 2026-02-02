#ifndef NETWORK_H
#define NETWORK_H

// --- Configuración de Red ---
#define PROTOCOL_VERSION    1       // Version del protocolo de red
#define GAME_PORT           8888    // Puerto TCP principal
#define DISCOVERY_PORT      8889    // Puerto UDP para broadcast
#define DISCOVERY_TIMEOUT   30      // Segundos para buscar partida
#define HOST_TIMEOUT        60      // Segundos que el host espera conexión
#define TIMEOUT_SEC         10      // Timeout de desconexión en partida (TCP)
#define HB_INTERVAL         2       // Intervalo de Heartbeat (segundos)
#define HB_MSG              0xFE    // Byte de Heartbeat (0xFE evita conflicto con -1)
#define DISCOVERY_MSG       "GATO_REQ"
#define DISCOVERY_ACK       "GATO_ACK"

// --- Descubrimiento Automático (LAN) ---

// HOST: Espera un cliente TCP, PERO también responde a búsquedas UDP.
// Reemplaza a net_start_server.
int net_host_wait_for_client(int port);

// CLIENTE: Busca automáticamente un host en la red por 'timeout_sec' segundos.
// Si lo encuentra, escribe la IP en 'found_ip' y devuelve 1. Si no, 0.
int net_discover_host(int port, int timeout_sec, char* found_ip);

// --- Utilidades de Juego ---
// Envía un voto (1=Si, 0=No) y recibe el del rival.
// Devuelve 1 si AMBOS quieren jugar, 0 en caso contrario.
int net_negotiate_rematch(int socket, int my_vote);

// Inicia la librería de Sockets de Windows (WSAStartup)
// Devuelve 1 si éxito, 0 si fallo.
int net_init(void);

// Limpia la librería al salir (WSACleanup)
void net_cleanup(void);

// --- Funciones para el HOST ---
// Crea un servidor que escucha en el puerto dado.
// Devuelve el "socket" (un número) de la conexión establecida o -1 si falla.
int net_start_server(int port);

// --- Funciones para el CLIENTE ---
// Se conecta a una IP y puerto específicos.
// Devuelve el "socket" de la conexión o -1 si falla.
int net_connect_to_host(const char* ip, int port);

// --- Comunicación ---
// Envía una jugada (fila y columna).
int net_send_move(int socket, int r, int c);

// Recibe una jugada (espera hasta que llegue).
// Retorna 1 si recibió bien, 0 si el otro se desconectó.
int net_receive_move(int socket, int *r, int *c);

// Cierra el socket de forma limpia
void net_close(int socket);

// Envía el nombre del jugador (fijo 32 bytes para simplicidad)
int net_send_name(int socket, const char* name);

// Recibe el nombre del rival
int net_receive_name(int socket, char* buffer);
#endif