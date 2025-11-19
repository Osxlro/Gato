#ifndef NETWORK_H
#define NETWORK_H

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

#endif