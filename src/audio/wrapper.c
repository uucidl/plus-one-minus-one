/* a10 291
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/audio/wrapper.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 291 */




#include <audio/wrapper.h>
#include <libc/stdlib.h>
#include <libc/math.h>

wrapper_t* wrapper_new(int bits)
{
    wrapper_t* wrap = calloc(1, sizeof(wrapper_t));

    wrap->bits = bits;
    
    return wrap;
}

int wrapper_destroy(wrapper_t* wrap)
{
    free(wrap);

    return 1;
}

void wrapper_copy(wrapper_t* dest, wrapper_t* source)
{
    dest->bits = source->bits;
    dest->fu.f = source->fu.f;
}

void wrapper_set(wrapper_t* wrap, double value)
{
    wrap->fu.f = value;
}

void wrapper_increment(wrapper_t* wrap, double increment)
{
    wrap->fu.f += increment;
}

double wrapper_get(wrapper_t* wrap)
{
    return (wrap)->fu.f;
}

double wrapper_get_wrap(wrapper_t* wrap) 
{
    return wrap->fu.f;// % (1<<wrap->bits);
}

int wrapper_get_int(wrapper_t* wrap)
{
    return (int)wrap->fu.f & ((1<<wrap->bits) - 1);
}

double wrapper_get_frac(wrapper_t* wrap)
{
    double i;

    return modf(wrap->fu.f, &i);
}
