/* a10 552
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/pan_portaudio_driver.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 552 */




#include "pan.h"
#include "pan_driver.h"
#include "effects.h"
#include "main.h"

#include <library/time.h>
#include <portaudio.h>
#include <libc/stdlib.h>

typedef struct pan_portaudio_driver_t
{
    pan_driver_t super;

    volatile int running_p;
    volatile int sample_rate;
    int channels;
    unsigned int frame_number;
    int buffer_number;
    volatile double start_ms;
    volatile int started_p;
    volatile int started_yeah_p;
    PaStream* stream;
    audio_effect_t* ae;

    PaDeviceIndex device_id;

    int (*original_configure_demo)(pan_driver_t* zelf, demo_t* demo);
} pan_portaudio_driver_t;

CLASS_INHERIT(pan_portaudio_driver, pan_driver);

static
int pan_portaudio_callback(const void* inputBuffer,
			   void* outputBuffer,
			   unsigned long framesPerBuffer,
			   const PaStreamCallbackTimeInfo* timeInfo,
			   PaStreamCallbackFlags flags,
			   void* userData)
{
    pan_portaudio_driver_t* self = (pan_portaudio_driver_t*) userData;
    double t = 1000.0 * timeInfo->outputBufferDacTime;

    if(self->started_p) {
	// do the thang.
	audio_effect_t* ae = self->ae;
	effect_t* e = &ae->super;

	if(!self->started_yeah_p) {
	    self->start_ms = t;
	    self->started_yeah_p = 1;
	}


	if(self->frame_number < framesPerBuffer) {
	    audio_area_audio_frame_signature_t signature;

	    audio_area_audio_frame_signature_instantiate_toplevel (&signature);

	    self->frame_number = framesPerBuffer;

	    signature.sample_rate = pan_get_sample_rate ();
	    signature.max_frame_number = self->frame_number;
	    signature.frame_size = 2;

	    ae->super.set_frame_signature (&ae->super, &signature.super);

	    audio_area_audio_frame_signature_retire (&signature);
	}

	{
	    audio_area_t area;

	    area.samples      = outputBuffer;
	    area.head         = 0;
	    area.frame_number = framesPerBuffer;

	    e->computes(e, &area, t - self->start_ms);
	}
    } else {
	   // outputs silence.
	   float* samples = outputBuffer;
       int i;
       self->start_ms = t;
       self->frame_number = framesPerBuffer;


	   for (i = 0; i < framesPerBuffer; i++) {
		  samples[2*i] = samples[2*i + 1] = 0.0f;
	   }
    }

    return 0;
}



static
int pan_portaudio_new(pan_driver_t* zelf, const char* device, int sr)
{
    pan_portaudio_driver_t* self = (pan_portaudio_driver_t*) zelf;

    self->running_p = 0;
    self->channels = 2;
    self->frame_number = 0;
    self->buffer_number = 4;
    self->start_ms = 0.0;
    self->started_p = 0;
    self->started_yeah_p = 0;

    if(Pa_Initialize() != paNoError)
	return 0;

    self->device_id = Pa_GetDefaultOutputDevice();
    if(self->device_id == paNoDevice)
	return 0;

    self->sample_rate = sr;

    return 1;
}

static
int pan_portaudio_configure_demo (pan_driver_t* zelf, demo_t* demo)
{
    pan_portaudio_driver_t* self = (pan_portaudio_driver_t*) zelf;
    PaError error;

    /* call super */
    if (!self->original_configure_demo (zelf, demo)) {
	return 0;
    }

    /* now the specific stuff */
    self->ae = (audio_effect_t*) demo->pan_effect_root;

    error = Pa_OpenDefaultStream(&self->stream, 0, self->channels, paFloat32,
	self->sample_rate,
	2.0 * self->sample_rate * demo->video_frame_ms / 1000.0,
	pan_portaudio_callback,
	self);

    if(error != paNoError)
	return 0;

    Pa_StartStream (self->stream);

    self->running_p = 1;

    return 1;
}

static
PaTime streamtime(pan_portaudio_driver_t* self)
{
    PaTime time = Pa_GetStreamTime(self->stream); //self->frame_number * self->buffer_number;
    return time < 0.0 ? 0.0 : time;
}

static
void pan_portaudio_start(pan_driver_t* zelf)
{
    pan_portaudio_driver_t* self = (pan_portaudio_driver_t*) zelf;

    if(self->running_p && !self->started_p) {
	double t = 0.0;

	if(self->frame_number > 0) {
	    audio_area_audio_frame_signature_t signature;

	    audio_area_audio_frame_signature_instantiate_toplevel (&signature);

	    signature.sample_rate      = self->sample_rate;
	    signature.max_frame_number = self->frame_number;
	    signature.frame_size       = 2;

	    self->ae->super
		.set_frame_signature (&self->ae->super, &signature.super);

	    audio_area_audio_frame_signature_retire (&signature);
	}

	self->started_p = 1;
	while(!self->started_yeah_p) {
	    Pa_Sleep(60);
	}
	do {
	    PaTime time = streamtime(self);
	    t = time * 1000.0;
	} while(t < self->start_ms);
    }
}

static
void pan_portaudio_stop(pan_driver_t* zelf)
{
    pan_portaudio_driver_t* self = (pan_portaudio_driver_t*) zelf;
    if(self->running_p) {
	Pa_StopStream(self->stream);
    }
}

static
long int pan_portaudio_get_fd(pan_driver_t* zelf)
{
    return -1;
}

static
double pan_portaudio_get_time(pan_driver_t* zelf)
{
    pan_portaudio_driver_t* self = (pan_portaudio_driver_t*) zelf;
    double t;

    if(!self->running_p) {
	t = get_milliseconds();
    } else {
	PaTime time = streamtime(self);
	t = time * 1000.0;
    }

    if(!self->started_p) {
	self->started_p = 1;
	self->start_ms = t;
    }

    t -= self->start_ms;

    return t <= 0.0 ? 0.0 : t;
}

static
int pan_portaudio_destroy(pan_driver_t* zelf)
{
    pan_portaudio_driver_t* self = (pan_portaudio_driver_t*) zelf;
    if(self->running_p) {
	Pa_CloseStream (self->stream);
	Pa_Terminate();
	self->running_p = 0;
    }

    return 1;
}

/* returns number of samples (1 for each frame) */
static
int pan_portaudio_get_samples_number(pan_driver_t* zelf)
{
    pan_portaudio_driver_t* self = (pan_portaudio_driver_t*) zelf;
    return self->frame_number;
}

static
int pan_portaudio_get_sample_rate(pan_driver_t* zelf)
{
    pan_portaudio_driver_t* self = (pan_portaudio_driver_t*) zelf;
    return self->sample_rate;
}

static
void pan_portaudio_update(pan_driver_t* zelf, sample_t* samples)
{
    // no op
}

/*
  always false, since updating is done outside of process
*/
static
int pan_portaudio_is_ready(pan_driver_t* zelf)
{
    return 0;
}

static
int pan_portaudio_return0(struct pan_driver_t* self)
{
    return 0;
}

pan_portaudio_driver_t* pan_portaudio_driver_instantiate(pan_portaudio_driver_t* x)
{
    pan_portaudio_driver_t* ppd = pan_portaudio_driver_instantiate_super (x);

    ppd->super.new                = pan_portaudio_new;
    ppd->super.destroy            = pan_portaudio_destroy;
    ppd->super.start              = pan_portaudio_start;
    ppd->super.stop               = pan_portaudio_stop;
    ppd->super.get_samples_number = pan_portaudio_get_samples_number;
    ppd->super.get_sample_rate    = pan_portaudio_get_sample_rate;
    ppd->super.has_fd             = pan_portaudio_return0;
    ppd->super.get_fd             = pan_portaudio_get_fd;
    ppd->super.is_ready           = pan_portaudio_is_ready;
    ppd->super.update             = pan_portaudio_update;
    ppd->super.get_time           = pan_portaudio_get_time;

    ppd->original_configure_demo = ppd->super.configure_demo;
    ppd->super.configure_demo	 = pan_portaudio_configure_demo;

    return ppd;
}

static
void pan_portaudio_driver_initialize() __attribute__((constructor));

static
void pan_portaudio_driver_initialize()
{
    put_pan_driver("portaudio", &pan_portaudio_driver_instantiate_toplevel(NULL)->super);
}
