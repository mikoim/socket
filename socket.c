#define _POSIX_C_SOURCE 200809L

#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>

int socket_version() {
    fprintf(stderr, "Socket Wrapper by Eshin Kunishima\nI love POSIX API. (%li)\n", _POSIX_C_SOURCE);
    return 0;
}

int socket_listen(char *service) {
    int sock, ret;
    struct addrinfo hints, *serverAddress;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((ret = getaddrinfo(NULL, service, &hints, &serverAddress)) != 0) {
        printf("socket_listen() -> getaddrinfo(): %s\n", gai_strerror(ret));
        return -1;
    }

    if ((sock = socket(serverAddress->ai_family, serverAddress->ai_socktype, serverAddress->ai_protocol)) < 0) {
        perror("socket_listen() -> socket()");
        freeaddrinfo(serverAddress);
        return -1;
    }

    /* Timeout */
    struct timeval timeout_tv;
    timeout_tv.tv_sec = 600;
    timeout_tv.tv_usec = 0;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout_tv, sizeof(struct timeval)) < 0) {
        perror("socket_listen() -> setsockopt(SO_RCVTIMEO)");
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout_tv, sizeof(struct timeval)) < 0) {
        perror("socket_listen() -> setsockopt(SO_SNDTIMEO)");
        return -1;
    }

    /* Bypass ANNOYING specification */
    int yes = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
        perror("socket_listen() -> setsockopt(SO_REUSEADDR)");
        return -1;
    }

    /*  */

    if (bind(sock, serverAddress->ai_addr, serverAddress->ai_addrlen) < 0) {
        perror("socket_listen() -> bind()");
        return -1;
    }

    if (listen(sock, 10) < 0) {
        perror("socket_listen() -> listen()");
        return -1;
    }

    freeaddrinfo(serverAddress);

    return sock;
}

int socket_accept(int sock) {
    int client;

    if ((client = accept(sock, NULL, NULL)) < 0) {
        perror("socket_accept() -> accept()");
        return -1;
    }

    return client;
}

int socket_connect(const char *hostname, char *service) {
    int sock, ret;
    struct addrinfo hints, *serverAddress, *cur;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if ((ret = getaddrinfo(hostname, service, &hints, &serverAddress)) != 0) {
        printf("connectServer() -> getaddrinfo(): %s\n", gai_strerror(ret));
        return -1;
    }

    sock = -1;

    for (cur = serverAddress; cur != NULL; cur = cur->ai_next) {
        if ((sock = socket(cur->ai_family, cur->ai_socktype, cur->ai_flags)) < 0) {
            perror("connectServer() -> socket()");
            sock = -1;
            continue;
        }

        if (connect(sock, cur->ai_addr, cur->ai_addrlen) < 0) {
            perror("connectServer() -> connect()");
            sock = -1;
        } else {
            // Establish the connection
            break;
        }
    }

    freeaddrinfo(serverAddress);

    return sock;
}

int socket_close(int sock) {
    if (shutdown(sock, SHUT_RDWR) < 0) {
        perror("socket_close() -> shutdown()");
        return -1;
    }

    if (close(sock) < 0) {
        perror("socket_close() -> close()");
        return -1;
    }

    return 0;
}

ssize_t socket_receive(int sock, char *buf, size_t size, int flags) {
    ssize_t receiveSize;

    if ((receiveSize = recv(sock, buf, size, flags)) < 0) {
        perror("socket_recv() -> recv()");
        return -1;
    }

    return receiveSize;
}

ssize_t socket_send(int sock, const char *data, size_t size, int flags) {
    ssize_t sendSize;

    if ((sendSize = send(sock, data, size, flags)) < 0) {
        perror("socket_send() -> send()");
        return -1;
    }

    return sendSize;
}