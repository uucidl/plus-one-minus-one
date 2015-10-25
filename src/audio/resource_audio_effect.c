/* a10 676
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/audio/resource_audio_effect.c') with
 *a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 676 */

#include "resource_audio_effect.h"
#include <scripting/dictionary.h>

static void resource_audio_effect_set_end_of_stream_receiver(
    resource_audio_effect_t *self, receiver_t *r, bytecode_stream_t *message)
{
    self->eos_rcv = r;
    self->eos_msg = message;
}

resource_audio_effect_t *
resource_audio_effect_instantiate(resource_audio_effect_t *x)
{
    dictionary_t *d = dictionary_get_instance();
    resource_audio_effect_t *rae = resource_audio_effect_instantiate_super(x);

    rae->set_end_of_stream_receiver =
        resource_audio_effect_set_end_of_stream_receiver;

    d->new_atom(d, "end-of-stream");

    return rae;
}
