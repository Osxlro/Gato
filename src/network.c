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

#include "network.h"
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

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

void net_cleanup(void) {
    WSACleanup();
}

// --- Lógica del Servidor (HOST) ---
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
        printf("Direccion IP invalida: %s\n", ip);
        closesocket(sock);
        return -1;
    }

    printf("Conectando a %s:%d...\n", ip, port);

    // 2. Conectar
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Error al conectar. ¿Esta el Host listo?\n");
        closesocket(sock);
        return -1;
    }

    printf("¡Conectado al Host!\n");
    return sock;
}

// --- Enviar / Recibir Jugadas ---

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