/* a10 399
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/libc/stdint.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 399 */



#ifndef KNOS_LIBC_STDINT_H
#define KNOS_LIBC_STDINT_H

#if defined(PS2)

typedef long int intptr_t;

typedef unsigned char uint8_t;

#if defined(EE)
typedef unsigned int		uint32_t;
typedef int			int32_t;
typedef long int		int64_t;
typedef unsigned long int	uint64_t;
#endif

#else

#include <stdint.h>

#endif

#endif
