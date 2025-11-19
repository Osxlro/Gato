/*
 * Nombre del archivo: src/network.c
 *
 * Responsabilidades:
 * - Implementar la lógica de bajo nivel de Winsock (Sockets de Windows).
 * - Manejar la inicialización (WSAStartup) y limpieza.
 * - Establecer conexiones TCP (Servidor/Listen y Cliente/Connect).
 * - Enviar y recibir datos crudos (bytes).
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Librerias del Juego
#include "network.h"

// Librerias del Lenguaje
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

/* ---- INICIALIZACION Y LIMPIEZA ---- */

// Inicializa Winsock. Necesario antes de usar cualquier función de red en Windows.
int net_init(void) {
    WSADATA wsaData;
    // Pedimos la versión 2.2 de Winsock
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        printf("Error al iniciar Winsock: %d\n", res);
        return 0;
    }
    return 1;
}

// Limpia Winsock. Llamar al finalizar la aplicación.
void net_cleanup(void) {
    WSACleanup();
}

// Cierra el socket de red.
void net_close(int socket) {
    if (socket >= 0) {
        shutdown(socket, SD_BOTH); // Avisa que no enviaremos más
        closesocket(socket);
    }
}

/* ---- LOGICA DE CONEXION ---- */

// Lógica del Servidor (HOST)
int net_start_server(int port) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);

    // 1. Crear el socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Error al crear socket: %d\n", WSAGetLastError());
        return -1;
    }

    // 2. Configurar la dirección (Cualquier IP local, Puerto X)
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // 3. Bind (Asociar socket al puerto)
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Error en bind (puerto %d ocupado?): %d\n", port, WSAGetLastError());
        closesocket(server_fd);
        return -1;
    }

    // 4. Listen (Esperar conexiones)
    if (listen(server_fd, 1) == SOCKET_ERROR) {
        printf("Error en listen.\n");
        closesocket(server_fd);
        return -1;
    }

    printf("Esperando rival en el puerto %d...\n", port);
    
    // 5. Accept (Bloquea hasta que alguien se conecte)
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) == INVALID_SOCKET) {
        printf("Error en accept.\n");
        closesocket(server_fd);
        return -1;
    }

    printf("¡Rival conectado!\n");
    
    // Ya no necesitamos el socket de escucha, solo el del cliente conectado
    closesocket(server_fd); 
    return client_fd;
}

// --- Lógica del Cliente (INVITADO) ---
int net_connect_to_host(const char* ip, int port) {
    int sock;
    struct sockaddr_in serv_addr;

    // 1. Crear socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Error creando socket.\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convertir IP texto a binario
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        printf("Direccion IP invalida\n");
        closesocket(sock);
        return -1;
    }

    printf("Conectando a %s:%d...\n", ip, port);

    // 2. Conectar
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Error al conectar. Esta el Host listo?\n");
        closesocket(sock);
        return -1;
    }

    printf("¡Conectado al Host!\n");
    return sock;
}

// --- IMPLEMENTACIÓN DE DESCUBRIMIENTO ---

// HOST: Escucha TCP y UDP al mismo tiempo
int net_host_wait_for_client(int tcp_port) {
    int tcp_fd, udp_fd, client_fd;
    struct sockaddr_in server_addr, client_addr, udp_addr, sender_addr;
    int addr_len = sizeof(client_addr);
    int sender_len = sizeof(sender_addr);
    fd_set readfds; // Conjunto de sockets a vigilar

    // 1. Configurar TCP (Igual que antes)
    if ((tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) return -1;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(tcp_port);
    if (bind(tcp_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) return -1;
    listen(tcp_fd, 1);

    // 2. Configurar UDP (Para ser descubierto)
    if ((udp_fd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) return -1;
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(DISCOVERY_PORT);
    // Permitir reutilizar puerto por si acaso
    char reuse = 1;
    setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    if (bind(udp_fd, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
        printf("Error bind UDP (puerto %d ocupado). El autodescubrimiento no funcionara.\n", DISCOVERY_PORT);
    }

    printf("Esperando rival (TCP %d) y escuchando busquedas (UDP %d)...\n", tcp_port, DISCOVERY_PORT);

    // 3. Bucle de espera (Multiplexación)
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(tcp_fd, &readfds);
        FD_SET(udp_fd, &readfds);

        // Esperar a que pase algo en alguno de los dos sockets
        if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) break;

        // CASO A: Alguien intenta conectarse por TCP (Juego)
        if (FD_ISSET(tcp_fd, &readfds)) {
            client_fd = accept(tcp_fd, (struct sockaddr *)&client_addr, &addr_len);
            printf("¡Rival conectado por TCP!\n");
            closesocket(tcp_fd); // Ya no escuchamos más
            closesocket(udp_fd); // Apagamos el beacon UDP
            return client_fd;
        }

        // CASO B: Alguien pregunta "¿Hay host?" por UDP
        if (FD_ISSET(udp_fd, &readfds)) {
            char buf[32];
            int n = recvfrom(udp_fd, buf, sizeof(buf), 0, (struct sockaddr*)&sender_addr, &sender_len);
            if (n > 0) {
                buf[n] = 0;
                if (strstr(buf, DISCOVERY_MSG)) {
                    // Responder al que preguntó: "Aquí estoy"
                    printf("Recibida solicitud de descubrimiento de %s. Respondiendo...\n", inet_ntoa(sender_addr.sin_addr));
                    sendto(udp_fd, DISCOVERY_ACK, strlen(DISCOVERY_ACK), 0, (struct sockaddr*)&sender_addr, sender_len);
                }
            }
        }
    }
    return -1;
}

// CLIENTE: Envía Broadcast y espera respuesta
int net_discover_host(int port, int timeout_sec, char* found_ip) {
    int sock;
    struct sockaddr_in broadcast_addr, sender_addr;
    int sender_len = sizeof(sender_addr);
    char buf[32];
    fd_set readfds;
    struct timeval tv;
    time_t start_time = time(NULL);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) return 0;

    // Habilitar Broadcast
    char broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(DISCOVERY_PORT);
    broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST; // 255.255.255.255

    printf("\nBuscando host en la red local (Max %d seg)...\n", timeout_sec);

    // Bucle de búsqueda hasta timeout
    while (difftime(time(NULL), start_time) < timeout_sec) {
        // 1. Enviar grito "¿Hay alguien?"
        sendto(sock, DISCOVERY_MSG, strlen(DISCOVERY_MSG), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));

        // 2. Esperar respuesta por 1 segundo
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        tv.tv_sec = 1; 
        tv.tv_usec = 0;

        int res = select(0, &readfds, NULL, NULL, &tv);
        if (res > 0) {
            int n = recvfrom(sock, buf, sizeof(buf)-1, 0, (struct sockaddr*)&sender_addr, &sender_len);
            if (n > 0) {
                buf[n] = 0;
                if (strstr(buf, DISCOVERY_ACK)) {
                    // ¡ENCONTRADO!
                    strcpy(found_ip, inet_ntoa(sender_addr.sin_addr));
                    printf("¡Host encontrado en %s!\n", found_ip);
                    closesocket(sock);
                    return 1;
                }
            }
        }
        printf("."); // Feedback visual
    }

    closesocket(sock);
    return 0; // Tiempo agotado
}

/* ---- ENVIO Y RECEPCION DE DATOS ---- */

// Envía 2 bytes simples: [fila, columna]
int net_send_move(int socket, int r, int c) {
    char buffer[2];
    buffer[0] = (char)r;
    buffer[1] = (char)c;
    
    int sent = send(socket, buffer, 2, 0);
    if (sent == SOCKET_ERROR) {
        printf("Error enviando datos.\n");
        return 0;
    }
    return 1;
}

// Recibe 2 bytes. Bloquea hasta recibir.
int net_receive_move(int socket, int *r, int *c) {
    char buffer[2];
    int valread = recv(socket, buffer, 2, 0);
    
    if (valread <= 0) {
        // 0 significa desconexión, <0 significa error
        return 0; 
    }
    
    *r = (int)buffer[0];
    *c = (int)buffer[1];
    return 1;
}

// Enviar Nombre (32 bytes cada uno)
int net_send_name(int socket, const char* name) {
    char buffer[32];
    // Aseguramos que el buffer esté limpio y copiamos el nombre
    memset(buffer, 0, 32);
    strncpy(buffer, name, 31);
    
    int sent = send(socket, buffer, 32, 0);
    return (sent != SOCKET_ERROR);
}

// Recibe Nombre (32 bytes cada uno)
int net_receive_name(int socket, char* buffer) {
    // Recibimos exactamente 32 bytes
    int valread = recv(socket, buffer, 32, 0);
    if (valread <= 0) return 0;
    
    // Aseguramos terminación nula por seguridad
    buffer[31] = '\0';
    return 1;
}

// Negociación de Revancha (1=Si, 0=No)
int net_negotiate_rematch(int socket, int my_vote) {
    // 1. Enviar mi voto
    char send_buf[2];
    send_buf[0] = my_vote ? 'Y' : 'N';
    send_buf[1] = '\0';
    send(socket, send_buf, 2, 0);

    // 2. Recibir voto rival
    char recv_buf[2];
    int n = recv(socket, recv_buf, 2, 0);
    
    if (n <= 0) return 0; // Error o desconexión
    
    int rival_vote = (recv_buf[0] == 'Y');

    return (my_vote && rival_vote);
}