#ifndef HEADER_SOCKET_H
#define HEADER_SOCKET_H

#include <unistd.h>
#include <sys/types.h>

typedef struct {
    struct addrinfo *address;
} SocketAddress;

typedef struct {
    int sockfd;
    int domain;
    int type;
} Socket;

enum {
    TYPE_NONE,
    TYPE_TCP,
    TYPE_UDP
};

enum {
    DOMAIN_NONE,
    DOMAIN_IPV4,
    DOMAIN_IPV6
};

int socket_address(SocketAddress *a, const char *hostname, int domain, int type);

void socket_address_print(const SocketAddress *address);

void socket_address_free(SocketAddress *address);

int socket_create(Socket *socket, int domain, int type);

int socket_timeout(Socket *socket, int sec);

int socket_bind(Socket *s, const char *service);

int socket_listen(Socket *s, int backlog);

int socket_connect(Socket *socket, const char *hostname, const char *service);

int socket_close(Socket *socket);

ssize_t socket_receive(Socket *socket, char *buf, size_t size, int flags);

ssize_t socket_send(Socket *socket, const char *data, size_t size, int flags);

ssize_t socket_recvfrom(Socket *socket, void *buf, size_t size, int flags, struct sockaddr *address, socklen_t *address_len);

ssize_t socket_sendto(Socket *socket, const void *data, size_t size, int flags, SocketAddress *address);

#endif