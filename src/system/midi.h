/* a10 633
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/midi.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 633 */

#ifndef KNOS_DEMOS_SYSTEM_MIDI_H
#define KNOS_DEMOS_SYSTEM_MIDI_H

#include <system/event_listener.h>

int midi_open();
int midi_close();
int midi_get_fd();
int midi_is_ready();
event_listener_t *midi_get_event_listener();
void midi_update();

#endif
