/* a10 873
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/libc/sys/types.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 873 */

#ifndef KNOS_LIBC_SYS_TYPES_H
#define KNOS_LIBC_SYS_TYPES_H

#include <sys/types.h>

#ifdef __MINGW32__
/* fd_set is defined there */
#include <libc/winsock.h>
#endif

#endif
