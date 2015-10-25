/* a10 358
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/libc/errno.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 358 */

#ifndef KNOS_LIBC_ERRNO_H
#define KNOS_LIBC_ERRNO_H

#include <errno.h>

#ifdef __MINGW32__

#define EISCONN WSAEISCONN - WSABASEERR
#define EINPROGRESS WSAEINPROGRESS - WSABASEERR
#define EALREADY WSAEALREADY - WSABASEERR
#define EWOULDBLOCK WSAEWOULDBLOCK - WSABASEERR

#endif

#endif
