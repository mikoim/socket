#ifndef HEADER_PACKET_H
#define HEADER_PACKET_H

ssize_t packet_send(int sock, void *data, size_t size);

ssize_t packet_receive(int sock, char *data);

#endif