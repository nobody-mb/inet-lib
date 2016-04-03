#include "tcp.h"

int _select_connect (int fd, struct addrinfo *result, time_t timeout)
{
	fd_set set;
	struct timeval tv;
	
	tv.tv_sec = timeout; 
	tv.tv_usec = 0;

	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		return -errno;

	if (connect(fd, result->ai_addr, result->ai_addrlen) < 0) {
		if (errno != EINPROGRESS)
			return -errno;
		
		FD_ZERO(&set); 
		FD_SET(fd, &set);
		
		if (select(fd + 1, NULL, &set, NULL, &tv) <= 0)
			return -errno;
	} 	
	
	if (fcntl(fd, F_SETFL, 0) < 0)
		return -errno;
	
	return fd;
}

int tcp_accept (const char *port)
{
	int fd, yes = 1;
	struct addrinfo hints, *result, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	if (getaddrinfo(NULL, port, &hints, &result) != 0) 
		return -errno;
	
	p = result;
	do {
		if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) >= 0) {
			if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
				return -errno;
			if (bind(fd, p->ai_addr, p->ai_addrlen) == 0)
				break;
		}
	} while ((p = p->ai_next) != NULL);
	
	freeaddrinfo(result);

	if (!p || listen(fd, 1) < 0)
		return -errno;

	if ((fd = accept(fd, NULL, NULL)) < 0)
		return -errno;
	
	return fd;
}

int tcp_connect (const char *host, const char *port, time_t timeout)
{
	int fd;
	struct addrinfo hints, *result, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	if (getaddrinfo(host, port, &hints, &result) != 0)
		return -errno;

	p = result;
	do {
		if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) >= 0)
			if (_select_connect(fd, p, timeout) >= 0)
				break;
	} while ((p = p->ai_next) != NULL);
	
	freeaddrinfo(result);
	
	return p ? fd : -errno;
}

int tcp_read (int fd, char *buf, size_t max, size_t chunk, size_t timeout)
{
	ssize_t res, pos = 0;
	fd_set set;
	struct timeval tv;
	
	tv.tv_sec = timeout; 
	tv.tv_usec = 0;
	
	FD_ZERO(&set); 
	FD_SET(fd, &set); 
	
	do {
		if ((res = select(fd + 1, &set, NULL, NULL, &tv)) <= 0)
			return !res ? (int)pos : -errno;
		if (FD_ISSET(fd, &set)) {
			res = read(fd, buf + pos, chunk);
			pos += res;
		}
	} while (pos < max && res > 0);
	
	return (int)pos;
}

int tcp_write (int fd, const char *buf, size_t len, size_t chunk)
{
	ssize_t res, pos = 0;
	
	do {
		res = write (fd, buf + pos, (len < chunk ? len : chunk));
		len -= res;
		pos += res;
	} while (len > 0 && res >= 0);
	
	return (int)pos;
}
