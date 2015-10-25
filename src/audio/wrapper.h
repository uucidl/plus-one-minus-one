/* a10 135
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/audio/wrapper.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 135 */

#ifndef KNOS_AUDIO_WRAPPER_H
#define KNOS_AUDIO_WRAPPER_H

#include <libc/endian.h>

/*
  a wrapper is a wraparound utility, associated with it is a size, and
  a current phase, which you can get the integer value, and the
  fractional value of.
*/
#define NORMALIZED_MSW 1094189056 /* normalized MSW for wrapping into 0...1 */
#define NORMALIZED_MSW_BIT                                                     \
    1048576 /* add for each bit of range more (2 ^ 20)                         \
               */
#define UNITBIT32 ((double)1572864.0) /* 3*2^19 -- bit 32 has value 1 */

typedef struct {
    int bits;
    union {
        double f;
        struct {
#if BYTE_ORDER == BIG_ENDIAN
            unsigned int hipart;
            unsigned int mantissa1;
#endif /* big endian */
#if BYTE_ORDER == LITTLE_ENDIAN
#if FLOAT_WORD_ORDER == BIG_ENDIAN
            unsigned int hipart;
            unsigned int mantissa1;
#else
            unsigned int mantissa1;
            unsigned int hipart;
#endif
#endif /* little endian */
        } ieee;
    } fu;
} wrapper_t;

wrapper_t *wrapper_new(int bits);
int wrapper_destroy(wrapper_t *w);
void wrapper_copy(wrapper_t *dest, wrapper_t *source);
void wrapper_set(wrapper_t *w, double value);
void wrapper_increment(wrapper_t *w, double increment);

/* wrap value then return
 * the order is important because the functions modify the wrapper object!
 * (int then frac)
 *
 * (use wrapper_copy to work on a temporary wrapper in your loop)
 */
double wrapper_get(wrapper_t *w);
double wrapper_get_wrap(wrapper_t *w);
int wrapper_get_int(wrapper_t *w);
double wrapper_get_frac(wrapper_t *w);

#endif
