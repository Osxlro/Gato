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
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        printf("Error al iniciar Winsock: %d\n", res);
        return 0;
    }
    return 1;
}

// Limpia Winsock.
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

    // MEJORA DE ROBUSTEZ: Timeout de recepción
    DWORD timeout = 60000; // 60 segundos
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    return sock;
}

/* --- IMPLEMENTACIÓN DE DESCUBRIMIENTO --- */

// HOST: Escucha TCP y UDP al mismo tiempo
int net_host_wait_for_client(int tcp_port) {
    int tcp_fd, udp_fd, client_fd;
    struct sockaddr_in server_addr, client_addr, udp_addr, sender_addr;
    int addr_len = sizeof(client_addr);
    int sender_len = sizeof(sender_addr);
    fd_set readfds;
    struct timeval tv; // Para no bloquear infinitamente
    time_t start_time = time(NULL); // Tiempo de inicio

    // 1. Configurar TCP
    if ((tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) return -1;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(tcp_port);
    if (bind(tcp_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) return -1;
    listen(tcp_fd, 1);

    // 2. Configurar UDP
    if ((udp_fd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) return -1;
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(DISCOVERY_PORT);
    char reuse = 1;
    setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    if (bind(udp_fd, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
        printf("Error bind UDP. Autodescubrimiento desactivado.\n");
    }

    printf("Esperando rival (Max %d s)...\n", HOST_TIMEOUT);

    // 3. Bucle de espera
    while (difftime(time(NULL), start_time) < HOST_TIMEOUT) { // 60 Segundos de Timeout GLOBAL
        FD_ZERO(&readfds);
        FD_SET(tcp_fd, &readfds);
        FD_SET(udp_fd, &readfds);

        // Timeout del select (1 segundo para revisar el bucle while)
        tv.tv_sec = 1; 
        tv.tv_usec = 0;

        int res = select(0, &readfds, NULL, NULL, &tv);

        if (res == SOCKET_ERROR) break;
        if (res == 0) continue; // Timeout de 1s, volvemos a verificar tiempo total

        // CASO A: TCP (Conexión entrante)
        if (FD_ISSET(tcp_fd, &readfds)) {
            client_fd = accept(tcp_fd, (struct sockaddr *)&client_addr, &addr_len);
            printf("¡Rival conectado por TCP!\n");
            
            // MEJORA DE ROBUSTEZ: Timeout de recepción (Si el rival se desconecta)
            DWORD timeout = 60000; // 60 segundos sin recibir nada = desconexión
            setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

            closesocket(tcp_fd);
            closesocket(udp_fd);
            return client_fd;
        }

        // CASO B: UDP (Búsqueda)
        if (FD_ISSET(udp_fd, &readfds)) {
            char buf[32];
            int n = recvfrom(udp_fd, buf, sizeof(buf), 0, (struct sockaddr*)&sender_addr, &sender_len);
            if (n > 0) {
                buf[n] = 0;
                if (strstr(buf, DISCOVERY_MSG)) {
                    sendto(udp_fd, DISCOVERY_ACK, strlen(DISCOVERY_ACK), 0, (struct sockaddr*)&sender_addr, sender_len);
                }
            }
        }
    }
    
    // Limpieza si se acaba el tiempo
    closesocket(tcp_fd);
    closesocket(udp_fd);
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

// Helper: Asegura que se envíen todos los bytes solicitados (maneja envíos parciales)
static int send_exact(int socket, const char *buf, int len) {
    int total = 0;
    int bytes_left = len;
    while (total < len) {
        int n = send(socket, buf + total, bytes_left, 0);
        if (n == SOCKET_ERROR) return 0;
        total += n;
        bytes_left -= n;
    }
    return 1;
}

// Helper: Asegura que se reciban todos los bytes solicitados (maneja fragmentación)
static int recv_exact(int socket, char *buf, int len) {
    int total = 0;
    time_t last_data = time(NULL);
    time_t last_hb   = time(NULL);

    while (total < len) {
        // 1. Verificar Timeout Global (Desconexion)
        if (difftime(time(NULL), last_data) > TIMEOUT_SEC) {
            printf("Error: Tiempo de espera agotado (Rival no responde).\n");
            return 0;
        }

        // 2. Enviar Heartbeat (Keep-Alive) mientras esperamos
        if (difftime(time(NULL), last_hb) >= HB_INTERVAL) {
            char hb = (char)HB_MSG;
            send(socket, &hb, 1, 0); 
            last_hb = time(NULL);
        }

        // 3. Select para no bloquear la UI por mucho tiempo
        fd_set fds; FD_ZERO(&fds); FD_SET(socket, &fds);
        struct timeval tv = {0, 100000}; // 100ms
        
        int res = select(0, &fds, NULL, NULL, &tv);
        if (res == SOCKET_ERROR) return 0;
        if (res == 0) continue; // Timeout del select, volvemos al loop

        // 4. Leer 1 byte
        char b;
        int n = recv(socket, &b, 1, 0);
        if (n <= 0) return 0;

        // 5. Filtrar Heartbeat
        if ((unsigned char)b == HB_MSG) {
            last_data = time(NULL); // Recibimos señal de vida
            continue;               // Ignoramos el byte, no es dato
        }

        buf[total++] = b;
        last_data = time(NULL);
    }
    return 1;
}

// Envía 2 bytes simples: [fila, columna]
int net_send_move(int socket, int r, int c) {
    char buffer[2];
    
    // Protocolo: Si r es -1 (Rendición), enviamos 0,0
    if (r == -1) {
        buffer[0] = 0;
        buffer[1] = 0;
    } else {
        buffer[0] = (char)r;
        buffer[1] = (char)c;
    }
    
    if (!send_exact(socket, buffer, 2)) {
        printf("Error enviando datos.\n");
        return 0;
    }
    return 1;
}

// Recibe 2 bytes. Bloquea hasta recibir.
int net_receive_move(int socket, int *r, int *c) {
    char buffer[2];
    
    // Usamos recv_exact para evitar leer solo 1 byte si hay fragmentación
    if (!recv_exact(socket, buffer, 2)) return 0;
    
    // Protocolo: Si recibimos 0,0 es Rendición
    if (buffer[0] == 0 && buffer[1] == 0) {
        *r = -1;
        *c = -1;
        return 1;
    }

    *r = (int)buffer[0];
    *c = (int)buffer[1];
    return 1;
}

// Enviar Nombre (32 bytes cada uno)
int net_send_name(int socket, const char* name) {
    // Enviamos [Version] + [Nombre (32)]
    char buffer[33];
    buffer[0] = (char)PROTOCOL_VERSION;
    
    memset(buffer + 1, 0, 32);
    if (name) strncpy(buffer + 1, name, 31);
    
    return send_exact(socket, buffer, 33);
}

// Recibe Nombre (32 bytes cada uno)
int net_receive_name(int socket, char* buffer) {
    char temp[33];
    // Recibimos 33 bytes (Version + Nombre)
    if (!recv_exact(socket, temp, 33)) return 0;
    
    if ((int)temp[0] != PROTOCOL_VERSION) {
        printf("Error: Version del juego incompatible (Rival: v%d, Local: v%d)\n", (int)temp[0], PROTOCOL_VERSION);
        return 0;
    }

    memcpy(buffer, temp + 1, 32);
    buffer[31] = '\0';
    return 1;
}

// Negociación de Revancha (1=Si, 0=No)
int net_negotiate_rematch(int socket, int my_vote) {
    // 1. Enviar mi voto
    char send_buf[2];
    send_buf[0] = my_vote ? 'Y' : 'N';
    send_buf[1] = '\0';
    send_exact(socket, send_buf, 2);

    // 2. Recibir voto rival
    char recv_buf[2];
    if (!recv_exact(socket, recv_buf, 2)) return 0;
    
    int rival_vote = (recv_buf[0] == 'Y');

    return (my_vote && rival_vote);
}