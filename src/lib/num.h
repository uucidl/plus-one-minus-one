/* a10 135
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/num.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 135 */



#include <libc/math.h>
	
static inline
int clamp(int x, int low, int high) 
{
    if(x <= low)
	return low;
    else if(x >= high)
	return high;
    else 
	return x;
}

	
