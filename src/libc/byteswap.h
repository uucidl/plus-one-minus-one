/* a10 937
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/libc/byteswap.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 937 */



#ifndef KNOS_LIBC_BYTESWAP_H
#define KNOS_LIBC_BYTESWAP_H

#include <libc/endian.h>

// Byte swapping functions from the GNU C Library and libsdl

/* Swap bytes in 16 bit value.  */
#ifdef __GNUC__
# define bswap_16(x) \
    (__extension__							      \
     ({ unsigned short int __bsx = (x);					      \
        ((((__bsx) >> 8) & 0xff) | (((__bsx) & 0xff) << 8)); }))
#else
static __inline unsigned short int
bswap_16 (unsigned short int __bsx)
{
  return ((((__bsx) >> 8) & 0xff) | (((__bsx) & 0xff) << 8));
}
#endif

/* Swap bytes in 32 bit value.  */
#ifdef __GNUC__
# define bswap_32(x) \
    (__extension__							      \
     ({ unsigned int __bsx = (x);					      \
        ((((__bsx) & 0xff000000) >> 24) | (((__bsx) & 0x00ff0000) >>  8) |    \
	 (((__bsx) & 0x0000ff00) <<  8) | (((__bsx) & 0x000000ff) << 24)); }))
#else
static __inline unsigned int
bswap_32 (unsigned int __bsx)
{
  return ((((__bsx) & 0xff000000) >> 24) | (((__bsx) & 0x00ff0000) >>  8) |
	  (((__bsx) & 0x0000ff00) <<  8) | (((__bsx) & 0x000000ff) << 24));
}
#endif

#if BYTE_ORDER == BIG_ENDIAN
#define bswapLE16(X) bswap_16(X)
#define bswapLE32(X) bswap_32(X)
#define bswapBE16(X) (X)
#define bswapBE32(X) (X)
#else
#define bswapLE16(X) (X)
#define bswapLE32(X) (X)
#define bswapBE16(X) bswap_16(X)
#define bswapBE32(X) bswap_32(X)
#endif

#endif
