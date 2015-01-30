#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <netinet/in.h>
#include "socket.h"
#include "packet.h"

ssize_t packet_send(Socket *s, void *data, size_t size) {
    ssize_t ret;
    char *packet;
    size_t packet_size;
    uint32_t data_size;

    packet_size = sizeof(uint32_t) + size;

    data_size = htonl((uint32_t) size);
    packet = calloc(1, packet_size);
#ifndef NDEBUG
    printf("packet_send(): DATA = %lu bytes / Packet = %lu bytes\n", size, packet_size);
#endif
    memcpy(packet, &data_size, sizeof(uint32_t));
    memcpy((packet + sizeof(uint32_t)), data, size);

    if (socket_send(s, packet, packet_size, 0) != packet_size)
        ret = -1;
    else
        ret = size;

    free(packet);

    return ret;
}

ssize_t packet_receive(Socket *s, char *data) {
    char buf[3000];
    size_t packet_size;
    uint32_t data_size;

    if (socket_receive(s, buf, sizeof(uint32_t), MSG_PEEK) != sizeof(uint32_t))
        return -1;

    data_size = ntohl(*(uint32_t *) buf);
    packet_size = sizeof(uint32_t) + data_size;
#ifndef NDEBUG
    printf("packet_receive(): DATA = %u bytes / Packet = %lu bytes\n", data_size, packet_size);
#endif
    if (socket_receive(s, buf, packet_size, 0) != packet_size)
        return -1;

    memcpy(data, (buf + sizeof(uint32_t)), data_size);

    return data_size;
}