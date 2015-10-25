/* a10 698
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/frame_converter.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 698 */

#include "frame_converter.h"

#include <scripting/dictionary.h>

frame_converter_frame_signature_t *frame_converter_frame_signature_instantiate(
    frame_converter_frame_signature_t *x)
{
    return frame_converter_frame_signature_instantiate_super(x);
}

static atom_t frame_type = 0;

static int frame_converter_new(frame_converter_t *self) { return 1; }

static int frame_converter_destroy(frame_converter_t *self) { return 1; }

static atom_t frame_converter_get_frame_type(frame_converter_t *self)
{
    return frame_type;
}

frame_converter_t *frame_converter_instantiate(frame_converter_t *x)
{
    frame_converter_t *fc = frame_converter_instantiate_super(x);

    if (!frame_type) {
        frame_type = dictionary_get_instance()->new_atom(
            dictionary_get_instance(), FRAME_CONVERTER_FRAME_TYPE_NAME);
    }

    fc->new = frame_converter_new;
    fc->destroy = frame_converter_destroy;
    fc->get_frame_type = frame_converter_get_frame_type;

    return fc;
}
