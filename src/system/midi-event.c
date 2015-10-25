/* a10 534
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/midi-event.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 534 */

#include "midi-event.h"
#include <scripting/dictionary.h>
#include <libc/stdlib.h>

midi_event_t *midi_event_instantiate(midi_event_t *x)
{
    dictionary_t *dict = dictionary_get_instance();
    midi_event_t *me = OBJECT_INSTANTIATE(midi_event, x);

    dict->new_atom(dict, "midi");
    dict->new_atom(dict, "channel");
    dict->new_atom(dict, "cc");
    dict->new_atom(dict, "noteon");
    dict->new_atom(dict, "noteoff");

    me->hierarchy = calloc(sizeof(bytecode_t), 6);
    me->size = 6;

    me->hierarchy[0].adverb = dict->get_atom(dict, "atom");
    me->hierarchy[0].verb = dict->get_atom(dict, "midi");

    return me;
}

midi_event_t *midi_cc_event_instantiate(midi_event_t *x)
{
    midi_event_t *me = midi_event_instantiate(x);
    dictionary_t *dict = dictionary_get_instance();

    me->hierarchy[1].adverb = dict->get_atom(dict, "atom");
    me->hierarchy[1].verb = dict->get_atom(dict, "channel");

    me->hierarchy[2].adverb = dict->get_atom(dict, "integer");
    me->hierarchy[2].verb = atom_new_integer(0);

    me->hierarchy[3].adverb = dict->get_atom(dict, "atom");
    me->hierarchy[3].verb = dict->get_atom(dict, "cc");

    me->hierarchy[4].adverb = dict->get_atom(dict, "integer");
    me->hierarchy[4].verb = atom_new_integer(0);

    me->hierarchy[5].adverb = dict->get_atom(dict, "integer");
    me->hierarchy[5].verb = atom_new_integer(0);

    return me;
}

midi_event_t *midi_noteon_event_instantiate(midi_event_t *x)
{
    midi_event_t *me = midi_event_instantiate(x);
    dictionary_t *dict = dictionary_get_instance();

    me->hierarchy[1].adverb = dict->get_atom(dict, "atom");
    me->hierarchy[1].verb = dict->get_atom(dict, "channel");

    me->hierarchy[2].adverb = dict->get_atom(dict, "integer");
    me->hierarchy[2].verb = atom_new_integer(0);

    me->hierarchy[3].adverb = dict->get_atom(dict, "atom");
    me->hierarchy[3].verb = dict->get_atom(dict, "noteon");

    me->hierarchy[4].adverb = dict->get_atom(dict, "integer");
    me->hierarchy[4].verb = atom_new_integer(0);

    me->hierarchy[5].adverb = dict->get_atom(dict, "integer");
    me->hierarchy[5].verb = atom_new_integer(0);

    return me;
}

midi_event_t *midi_noteoff_event_instantiate(midi_event_t *x)
{
    midi_event_t *me = midi_event_instantiate(x);
    dictionary_t *dict = dictionary_get_instance();

    me->hierarchy[1].adverb = dict->get_atom(dict, "atom");
    me->hierarchy[1].verb = dict->get_atom(dict, "channel");

    me->hierarchy[2].adverb = dict->get_atom(dict, "integer");
    me->hierarchy[2].verb = atom_new_integer(0);

    me->hierarchy[3].adverb = dict->get_atom(dict, "atom");
    me->hierarchy[3].verb = dict->get_atom(dict, "noteoff");

    me->hierarchy[4].adverb = dict->get_atom(dict, "integer");
    me->hierarchy[4].verb = atom_new_integer(0);

    me->hierarchy[5].adverb = dict->get_atom(dict, "integer");
    me->hierarchy[5].verb = atom_new_integer(0);

    return me;
}
