#ifndef HEADER_SOCKET_H
#define HEADER_SOCKET_H

#include <unistd.h>
#include <sys/types.h>

int socket_version();

int socket_listen(char *service);

int socket_connect(const char *hostname, char *service);

int socket_accept(int sock);

int socket_close(int sock);

ssize_t socket_receive(int sock, char *buf, size_t size, int flags);

ssize_t socket_send(int sock, const char *data, size_t size, int flags);

#endif