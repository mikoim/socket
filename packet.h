#ifndef HEADER_PACKET_H
#define HEADER_PACKET_H

ssize_t packet_send(Socket *s, void *data, size_t size);

ssize_t packet_receive(Socket *s, char *data);

#endif