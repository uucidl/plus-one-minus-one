/* a10 291
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/sock.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 291 */

#ifndef KNOS_LIBRARY_SOCK_H
#define KNOS_LIBRARY_SOCK_H

#include <libc/sys/types.h>
#include <libc/sys/socket.h>
#include <libc/netinet/in.h>
#include <libc/errno.h>
#include <libc/unistd.h>
#include <libc/netdb.h>

int sock_open(int domain, int type, int protocol);
void sock_set_nonblocking(int socket);
int sock_get_recv_buffer_size(int socket);
int sock_get_send_buffer_size(int socket);
int sock_connect(int socket, struct sockaddr *server_addr, socklen_t addrlen);
int sock_close(int socket);
ssize_t sock_read(int socket, void *buffer, size_t size);
ssize_t sock_write(int socket, const void *buffer, size_t size);
int sock_errno();
void sock_perror(const char *message);

#endif
