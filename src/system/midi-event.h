/* a10 971
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/midi-event.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 971 */



#ifndef KNOS_DEMOS_SYSTEM_MIDI_EVENT_H
#define KNOS_DEMOS_SYSTEM_MIDI_EVENT_H

#include "event.h"

typedef event_t midi_event_t;
typedef event_t midi_cc_event_t;
typedef event_t midi_noteon_event_t;
typedef event_t midi_noteoff_event_t;
/* with
   hierarchy atom:midi 
   (atom:channel integer:<channel_number>
    (atom:cc integer:<controler> integer:<value> |
     atom:noteon integer:<note> integer:<velocity> |
     atom:noteoff integer:<note> (integer:<velocity> | nothing)))
*/

CLASS_INHERIT(midi_event, object);

static inline
object_t* midi_cc_event_to_object(midi_cc_event_t* x) {
  return event_to_object(x);
}

static inline
object_t* midi_noteon_event_to_object(midi_noteon_event_t* x) {
  return event_to_object(x);
}

static inline
object_t* midi_noteoff_event_to_object(midi_noteoff_event_t* x) {
  return event_to_object(x);
}

DEFINE_OBJECT_INSTANTIATE(midi_cc_event);
DEFINE_OBJECT_INSTANTIATE(midi_noteon_event);
DEFINE_OBJECT_INSTANTIATE(midi_noteoff_event);

#endif
