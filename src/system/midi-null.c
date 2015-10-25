/* a10 391
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/midi-null.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 391 */

#include "midi.h"

/*
   midi null driver
*/

#include <log4c.h>
// LOG_NEW_DEFAULT_SUBCATEGORY(MIDI, KNOS_DEMOS_SYSTEM);

static int midi_running_p;

#include "demo.h"

int midi_open() { return (midi_running_p = 1); }

int midi_close() { return 1; }

int midi_get_fd() { return -1; }

int midi_is_ready() { return 0; }

#include "midi-event.h"
#include <libc/stdlib.h>

static event_listener_t *event_listener = NULL;

event_listener_t *midi_get_event_listener()
{
    if (!event_listener)
        event_listener = event_listener_instantiate_toplevel(NULL);

    return event_listener;
}

void midi_update()
{
    // no-op
}
