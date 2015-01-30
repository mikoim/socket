#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include "socket.h"

int socket_address(SocketAddress *a, const char *hostname, int domain, int type) {
    int ret, family, socktype, protocol;
    struct addrinfo hints;

    switch (domain) {
        case DOMAIN_NONE:
            family = AF_UNSPEC;
            break;

        case DOMAIN_IPV4:
            family = AF_INET;
            break;

        case DOMAIN_IPV6:
            family = AF_INET6;
            break;

        default:
            fprintf(stderr, "socket_address(): Unknown domain\n");
            return -1;
    }

    switch (type) {
        case TYPE_NONE:
            socktype = 0;
            protocol = 0;
            break;

        case TYPE_TCP:
            socktype = SOCK_STREAM;
            protocol = IPPROTO_TCP;
            break;

        case TYPE_UDP:
            socktype = SOCK_DGRAM;
            protocol = IPPROTO_UDP;
            break;

        default:
            fprintf(stderr, "socket_address(): Unknown type\n");
            return -1;
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = family;
    hints.ai_flags = 0;
    hints.ai_protocol = protocol;
    hints.ai_socktype = socktype;

    if ((ret = getaddrinfo(hostname, NULL, &hints, &a->address)) != 0) {
        printf("socket_connect() -> getaddrinfo(): %s\n", gai_strerror(ret));
        return -1;
    }

    return 0;
}

void socket_address_print(const SocketAddress *a) {
    int count;
    struct addrinfo *cur;
    char str[INET6_ADDRSTRLEN];
    void *address;

    for (cur = a->address, count = 0; cur != NULL; cur = cur->ai_next, count++) {
        switch (cur->ai_addr->sa_family) {
            case AF_INET:
                address = &((struct sockaddr_in *) cur->ai_addr)->sin_addr;
                break;

            case AF_INET6:
                address = &((struct sockaddr_in6 *) cur->ai_addr)->sin6_addr;
                break;

            default:
                address = NULL;
                break;
        }

        if (inet_ntop(cur->ai_addr->sa_family, address, str, INET6_ADDRSTRLEN) == NULL) {
            perror("socket_address_print() -> inet_ntop()");
        } else {
            printf("[%d] %s\n", count, str);
        }
    }
}

void socket_address_free(SocketAddress *a) {
    freeaddrinfo(a->address);
}

int socket_create(Socket *s, int domain, int type) {
    int yes = 1;

    switch (domain) {
        case DOMAIN_IPV4:
            s->domain = AF_INET;
            break;

        case DOMAIN_IPV6:
            s->domain = AF_INET6;
            break;

        default:
            fprintf(stderr, "socket_create(): Unknown domain\n");
            return -1;
    }

    switch (type) {
        case TYPE_TCP:
            s->type = SOCK_STREAM;
            break;

        case TYPE_UDP:
            s->type = SOCK_DGRAM;
            break;

        default:
            fprintf(stderr, "socket_create(): Unknown type\n");
            return -1;
    }

    if ((s->sockfd = socket(s->domain, s->type, 0)) < 0) {
        perror("socket_create() -> socket()");
        return -1;
    }

    if (setsockopt(s->sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
        perror("socket_create() -> setsockopt(SO_REUSEADDR)");
        return -1;
    }

    return 0;
}

int socket_timeout(Socket *s, int sec) {
    struct timeval timeout_tv;
    timeout_tv.tv_sec = sec;
    timeout_tv.tv_usec = 0;

    if (setsockopt(s->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout_tv, sizeof(struct timeval)) < 0) {
        perror("socket_timeout() -> setsockopt(SO_RCVTIMEO)");
        return -1;
    }

    if (setsockopt(s->sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout_tv, sizeof(struct timeval)) < 0) {
        perror("socket_timeout() -> setsockopt(SO_SNDTIMEO)");
        return -1;
    }

    return 0;
}

int socket_listen(Socket *s, const char *service, int backlog) {
    int ret;
    struct addrinfo hints, *serverAddress;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = s->domain;
    hints.ai_socktype = s->type;

    if ((ret = getaddrinfo(NULL, service, &hints, &serverAddress)) != 0) {
        printf("socket_listen() -> getaddrinfo(): %s\n", gai_strerror(ret));
        return -1;
    }

    if (bind(s->sockfd, serverAddress->ai_addr, serverAddress->ai_addrlen) < 0) {
        perror("socket_listen() -> bind()");
        return -1;
    }

    if (listen(s->sockfd, backlog) < 0) {
        perror("socket_listen() -> listen()");
        return -1;
    }

    freeaddrinfo(serverAddress);

    return 0;
}

int socket_connect(Socket *s, const char *hostname, const char *service) {
    int ret;
    struct addrinfo hints, *serverAddress, *cur;

    memset(s, 0, sizeof(Socket));
    s->domain = AF_UNSPEC;
    s->type = SOCK_STREAM;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = s->domain;
    hints.ai_flags = 0;
    hints.ai_socktype = s->type;
    hints.ai_protocol = IPPROTO_TCP;

    if ((ret = getaddrinfo(hostname, service, &hints, &serverAddress)) != 0) {
        printf("socket_connect() -> getaddrinfo(): %s\n", gai_strerror(ret));
        return -1;
    }

    for (cur = serverAddress; cur != NULL; cur = cur->ai_next) {
        if ((s->sockfd = socket(cur->ai_family, cur->ai_socktype, cur->ai_flags)) < 0) {
            perror("socket_connect() -> socket()");
            s->sockfd = -1;
            continue;
        }

        if (connect(s->sockfd, cur->ai_addr, cur->ai_addrlen) < 0) {
            perror("socket_connect() -> connect()");
            s->sockfd = -1;
        } else {
            s->domain = cur->ai_family;
            break;
        }
    }

    freeaddrinfo(serverAddress);

    if (s->sockfd == -1)
        return -1;

    return 0;
}

int socket_close(Socket *s) {
    if (shutdown(s->sockfd, SHUT_RDWR) < 0) {
        perror("socket_close() -> shutdown()");
        return -1;
    }

    if (close(s->sockfd) < 0) {
        perror("socket_close() -> close()");
        return -1;
    }

    return 0;
}

ssize_t socket_receive(Socket *s, char *buf, size_t size, int flags) {
    ssize_t receiveSize;

    if ((receiveSize = recv(s->sockfd, buf, size, flags)) < 0) {
        perror("socket_recv() -> recv()");
        return -1;
    }

    return receiveSize;
}

ssize_t socket_send(Socket *s, const char *data, size_t size, int flags) {
    ssize_t sendSize;

    if ((sendSize = send(s->sockfd, data, size, flags)) < 0) {
        perror("socket_send() -> send()");
        return -1;
    }

    return sendSize;
}