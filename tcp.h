#ifndef INET_LIB_TCP_H
#define INET_LIB_TCP_H

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/select.h>

#include <netdb.h>
#include <fcntl.h>

int tcp_accept (const char *port);
int tcp_connect (const char *host, const char *port, time_t timeout);

int tcp_read (int fd, char *buf, size_t max, size_t chunk, size_t timeout);
int tcp_write (int fd, const char *buf, size_t len, size_t chunk);

#endif