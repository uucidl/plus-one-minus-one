/* a10 651
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/metro.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 651 */




#include <libc/stdlib.h>
#include <libc/math.h>
#include "metro.h"

static
void beatsource_set_start(beatsource_t* self, double startms)
{
    self->start_ms = startms;
}

static
metro_event_t* beatsource_pump(beatsource_t* self, double ms)
{
    return NULL;
}

static
double beatsource_predict(beatsource_t* self, double ms)
{
    return +HUGE_VAL;
}

beatsource_t* beatsource_instantiate(beatsource_t* x)
{
    beatsource_t* b = beatsource_instantiate_super (x);

    b->set_start = beatsource_set_start;
    b->pump	 = beatsource_pump;
    b->predict	 = beatsource_predict;

    return b;
}

static
void metronome_init(metronome_t* self, 
		    double startms, double periodms, double resolutionms)
{
    if(periodms <= 0 || resolutionms <= 0)
	return;
    
    self->super.start_ms = startms;
    self->last_beat     = -1;
    self->period_ms     = periodms;
    self->resolution_ms = resolutionms;
}

static
int metronome_new(metronome_t* self, 
		  double startms, double periodms, double resolutionms)
{
    if(periodms <= 0 || resolutionms <= 0)
	return 0;

    metronome_init(self, startms, periodms, resolutionms);

    return 1;
}

static
metro_event_t* metronome_pump(beatsource_t* self, double ms)
{
    metronome_t* m = (metronome_t*) self;
    metro_event_t* ret = NULL;

    double n = floor( (ms-self->start_ms) / m->period_ms);
    double r = ms - m->last_beat * m->period_ms;

    if(n <= m->last_beat &&
       (r < -m->resolution_ms || r > m->resolution_ms))
	return ret;
    else {
	m->last_beat   = n;

	m->tick.number = n;
	m->tick.ms     = m->last_beat * m->period_ms;

	return &m->tick;
    }
}

static
double metronome_predict(beatsource_t* zelf, double now)
{
    metronome_t* self = (metronome_t*) zelf;
    double n = floor( (now - zelf->start_ms) / self->period_ms ) + 1;

    return n * self->period_ms;
}

metronome_t* metronome_instantiate(metronome_t* x)
{
    metronome_t* m = metronome_instantiate_super (x);

    m->new	     = metronome_new;
    m->reinit	     = metronome_init;
    m->super.pump    = metronome_pump;
    m->super.predict = metronome_predict;

    return m;
}

int swinger_new(swinger_t* self, 
		unsigned int beats, beatsource_t* beatsource)
{
    if(beats <= 0 || beatsource == NULL)
	return 0;

    self->times = calloc(beats, sizeof(unsigned int));
    self->beats = beats;
    self->master = beatsource;
    self->master_last = -1;
    
    return 1;
}

void swinger_set_beat(swinger_t* self,
		      unsigned int beat, unsigned int times)
{
    if(beat < self->beats) {
	if(times < 1) 
	    times = 1;

	self->times[beat] = times;
    }
}

metro_event_t* swinger_pump(beatsource_t* self, double ms)
{
    swinger_t* s = (swinger_t*) self;
    metro_event_t*   e;
    metro_event_t*   res = NULL;
    
    if(!s->initialized_p) {
	s->current = 0;
	s->counter = s->times[0];
	s->initialized_p = 1;
    }
    
    e = s->master->pump(s->master, ms); 
    
    if(e) {
	int diff = e->number - s->master_last;
	s->master_last = e->number;

	while(diff-- > 0) {
	    if(s->counter) s->counter--;
	    if(s->counter == 0) {
		s->tick.number++;
		s->tick.ms = e->ms;

		s->current = (s->current + 1) % s->beats;
		s->counter = s->times[s->current];
		
		res = &s->tick;
	    }
	}
    }

    return res;
}

static
double swinger_predict(beatsource_t* zelf, double now)
{
    swinger_t* self = (swinger_t*) zelf;
    /* number of times we need to consume beats to get the prediction */
    const int n = 1 + self->counter; 
    double ms = now;
    int i;

    for(i=0; i<n; i++) {
	ms = self->master->predict(self->master, ms); 
    }

    return ms;
}

swinger_t* swinger_instantiate(swinger_t* x)
{
    swinger_t* s = swinger_instantiate_super (x);

    s->new	     = swinger_new;
    s->set_beat	     = swinger_set_beat;
    s->super.pump    = swinger_pump;
    s->super.predict = swinger_predict;

    return s;
}
    
