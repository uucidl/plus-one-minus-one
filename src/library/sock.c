/* a10 806
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/sock.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 806 */




#include <library/sock.h>

#include <libc/stdio.h>
#include <libc/signal.h>
#include <fcntl.h>
#include <unistd.h>

static
void sigpipe_block() __attribute__((constructor));

static
void sigpipe_block()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);

    sigprocmask(SIG_BLOCK, &set, NULL);
}

int sock_open(int domain, int type, int protocol)
{
    int sock;

    errno = 0;
    do {
	sock = socket(domain, type, protocol);
    } while(sock < 0 && errno == EINTR);

    return sock;
}

void sock_set_nonblocking(int socket)
{
    int flags = fcntl(socket, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(socket, F_SETFL, flags);
}

int sock_get_recv_buffer_size(int socket)
{
    size_t pom = 0;
    socklen_t len = 0;

    if(getsockopt(socket, SOL_SOCKET, SO_RCVBUF, &pom, &len) < 0) {
	return -1;
    } else
	return pom;
}

int sock_get_send_buffer_size(int socket)
{
    size_t pom = 0;
    socklen_t len = 0;

    if(getsockopt(socket, SOL_SOCKET, SO_SNDBUF, &pom, &len) < 0) {
	return -1;
    } else {
	return pom;
    }
}

int sock_connect(int socket, struct sockaddr* server_addr, socklen_t addrlen)
{
    int err;

    errno = 0;
    do {
	err = connect(socket, server_addr, addrlen);
    } while(err < 0 && errno == EINTR);

    return err;
}

int sock_close(int socket)
{
    int err;

    errno = 0;
    do {
	err = close(socket);
    } while(err < 0 && errno == EINTR);

    return err;
}

ssize_t sock_read(int socket, void* buffer, size_t size)
{
    int nbytes;

    errno = 0;
    do {
	nbytes = read(socket, buffer, size);
    } while(nbytes < 0 && errno == EINTR);

    return nbytes;
}

ssize_t sock_write(int socket, const void* buffer, size_t size)
{
    int nbytes;

    errno = 0;
    do {
	nbytes = write(socket, buffer, size);
    } while(nbytes < 0 && errno == EINTR);

    return nbytes;
}

int sock_errno()
{
    return errno;
}

void sock_perror(const char* msg)
{
    perror(msg);
}
