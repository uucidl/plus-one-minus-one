/* a10 572
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/libc/endian.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 572 */



#ifndef KNOS_LIBC_ENDIAN_H
#define KNOS_LIBC_ENDIAN_H

/* pixel endianness is also specified here

   PIXEL_RGBA8888 means:
     bit 0...8...16...24...32
	 R   G   B     A

   PIXEL BGRA8888 means:
     bit 0...8...16...24...32
	 B   G   R     A

*/



#if defined (LINUX)
  #include <endian.h>

#ifndef LITTLE_ENDIAN
#  define LITTLE_ENDIAN	__LITTLE_ENDIAN
#endif

#ifndef BIG_ENDIAN
#  define BIG_ENDIAN	__BIG_ENDIAN
#endif

#ifndef PDP_ENDIAN
#  define PDP_ENDIAN	__PDP_ENDIAN
#endif

#ifndef BYTE_ORDER
#  define BYTE_ORDER __BYTE_ORDER
#endif

#ifndef FLOAT_WORD_ORDER
#  define FLOAT_WORD_ORDER __FLOAT_WORD_ORDER
#endif

#ifndef PIXEL_RGBA8888
#  define PIXEL_RGBA8888 __PIXEL_RGBA
#endif

#elif defined(MACOSX)
  #include <machine/endian.h>

#ifndef FLOAT_WORD_ORDER
#  define FLOAT_WORD_ORDER BYTE_ORDER
#endif

#ifndef PIXEL_RGBA8888
#  define PIXEL_RGBA8888 __PIXEL_RGBA
#endif

#elif defined(WIN32)

#ifndef LITTLE_ENDIAN
  #define LITTLE_ENDIAN 0
#endif
#ifndef BIG_ENDIAN
  #define BIG_ENDIAN 1
#endif

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#ifndef FLOAT_WORD_ORDER
#define FLOAT_WORD_ORDER BYTE_ORDER
#endif

#ifndef PIXEL_RGBA8888
#  define PIXEL_RGBA8888 __PIXEL_RGBA
#endif

#elif defined(PS2)

#ifndef LITTLE_ENDIAN
  #define LITTLE_ENDIAN 0
#endif
#ifndef BIG_ENDIAN
  #define BIG_ENDIAN 1
#endif

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#ifndef FLOAT_WORD_ORDER
#define FLOAT_WORD_ORDER BYTE_ORDER
#endif

#ifndef PIXEL_BGRA8888
#  define PIXEL_BGRA8888 __PIXEL_BGRA
#endif

#else
#error please define endianness for your platform
#endif

#endif
