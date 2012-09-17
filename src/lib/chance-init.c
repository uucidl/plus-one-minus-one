/* a10 412
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/chance-init.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 412 */




#include "chance.h"

#include <library/time.h>
#include <lib/cokus.h>

#include <libc/stdio.h>

static
void reinit () __attribute__((constructor));

static
void reinit()
{
	printf ("reinit: hello\n");
	static int chance_init_flag = 0;
	if (!chance_init_flag) {
		chance_init_flag = 1;
		
		const float t = (get_unix_milliseconds() / 1000.) + get_milliseconds() / 1000.;
		const float frac = t - rint (1000000. * t) / 1000000.;
		const unsigned int SEED = (unsigned int)  (frac * UINT_MAX);
		
		seedMT (SEED);
	}
}

