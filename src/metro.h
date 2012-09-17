/* a10 410
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/metro.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 410 */



#ifndef KNOS_DEMOS_1_1_METRO_H
#define KNOS_DEMOS_1_1_METRO_H

#include <library/memory.h>
#include <scripting/atom.h>

/*
  rythm / tempo objects
*/

typedef struct
{
    object_t super;
    atom_t   number; /* event number */
    double ms;     /* event time   */
} metro_event_t;

CLASS_INHERIT(metro_event, object)

typedef struct beatsource_t 
{
    object_t super;
    void (*set_start)(struct beatsource_t* self, double startms);

    /* return NULL if no beat,
     * and update to with corresponding event if !NULL.
     */
    metro_event_t* (*pump)(struct beatsource_t* self, double ms);
    
    /*
     * return ms of next event > provided ms
     */
    double (*predict)(struct beatsource_t* self, double ms);

    /* the beat source's origin is start_ms */
    double start_ms;
} beatsource_t;

CLASS_INHERIT(beatsource, object)

typedef struct metronome_t {
    beatsource_t super;
    
    int  (*new)(struct metronome_t* self, 
		double startms, double periodms, double resolutionms);
    void (*reinit)(struct metronome_t* self, 
		   double startms, double periodms, double resolutionms);
    double period_ms;
    double resolution_ms;
    int last_beat; /* last played beat */
    metro_event_t tick;
} metronome_t;

CLASS_INHERIT(metronome, beatsource)

/*
  non linear metronome
*/
typedef struct swinger_t
{
    beatsource_t super;

    int  (*new)(struct swinger_t* self, 
		unsigned int beats, beatsource_t* beatsource);
    /* beat's origin is 0 */
    void (*set_beat)(struct swinger_t* self, 
		     unsigned int beat, unsigned int times);
    
    /* master beatsource */
    beatsource_t* master;
    int           master_last;

    /* number of times per beat */
    unsigned int* times;
    unsigned int  beats; /* number of beats */

    /* current */
    unsigned int counter; /* number of times still to count */
    unsigned int current; /* current beat */
    metro_event_t tick; /* last sent tick */
    unsigned int initialized_p;
} swinger_t;

CLASS_INHERIT(swinger, beatsource)

#endif
