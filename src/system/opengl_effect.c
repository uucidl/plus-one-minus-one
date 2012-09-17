/* a10 667
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/opengl_effect.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 667 */




#include "opengl_effect.h"

static atom_t frame_type = 0;

static
atom_t opengl_effect_get_frame_type(effect_t* self)
{
    return frame_type;
}

opengl_effect_t* opengl_effect_instantiate(opengl_effect_t* x)
{
    opengl_effect_t* oe = opengl_effect_instantiate_super (x);

    if(!frame_type) {
	frame_type = dictionary_get_instance()->new_atom(dictionary_get_instance(), OPENGL_FRAME_TYPE_NAME);
    }

    oe->super.get_frame_type = opengl_effect_get_frame_type;

    return oe;
}
