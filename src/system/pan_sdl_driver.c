/* a10 982
 * Copyright (c) 2014 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/pan_sdl_driver.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 982 */

#include "pan_driver.h"

#include <SDL.h>
#include <stdlib.h>

typedef struct pan_sdl_driver_t {
    pan_driver_t super;
    int (*core_configure_demo)(struct pan_driver_t *self, demo_t *demo);

    SDL_AudioSpec spec;
    effect_t *effect;
    double time;
    float *buffer;
} pan_sdl_driver_t;

CLASS_INHERIT(pan_sdl_driver, pan_driver);

static void SDL_AudioCallback(void *userdata, Uint8 *stream, int len)
{
    pan_sdl_driver_t *self = userdata;

    int16_t *ibuffer = (int16_t *)stream;
    int const samples_number = len / sizeof *ibuffer;
    int const frame_number = samples_number / self->spec.channels;

    audio_area_t area;
    area.samples = self->buffer;
    area.head = 0;
    area.frame_number = frame_number;

    self->effect->computes(self->effect, &area, self->time);

    for (int i = 0; i < samples_number; i++) {
        ibuffer[i] = (0x7fff - 1) * self->buffer[i];
    }
    self->time += 1000.0 * area.frame_number / (double)self->spec.freq;
}

static int pan_sdl_new(struct pan_driver_t *zelf, const char *device,
                       int sample_rate)
{
    pan_sdl_driver_t *self = (pan_sdl_driver_t *)zelf;

    (void)device;
    (void)sample_rate;

    if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
        if (SDL_WasInit(0) == 0) {
            SDL_Init(0);
        }

        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            return 0;
        }
    }

    SDL_AudioSpec desired = {0};

    desired.freq = 44100;
    desired.format = AUDIO_S16SYS;
    desired.channels = 2;
    desired.samples = 4096;
    desired.callback = SDL_AudioCallback;
    desired.userdata = self;

    if (SDL_OpenAudio(&desired, &self->spec) < 0) {
        return 0;
    }

    if (self->spec.format != AUDIO_S16SYS) {
        return 0;
    }

    return 1;
}

static int pan_sdl_destroy(struct pan_driver_t *zelf)
{
    pan_sdl_driver_t *self = (pan_sdl_driver_t *)zelf;

    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    free(self->buffer);

    return 1;
}

static void pan_sdl_start(pan_driver_t *zelf)
{
    pan_sdl_driver_t *self = (pan_sdl_driver_t *)zelf;

    const int sample_count = self->spec.samples * self->spec.channels;
    self->buffer = calloc(sizeof *self->buffer, sample_count);

    SDL_PauseAudio(0);
}

static void pan_sdl_stop(pan_driver_t *zelf)
{
    pan_sdl_driver_t *self = (pan_sdl_driver_t *)zelf;

    SDL_PauseAudio(1);
    SDL_CloseAudio();
}

static int pan_sdl_get_samples_number(pan_driver_t *zelf)
{
    pan_sdl_driver_t *self = (pan_sdl_driver_t *)zelf;
    return self->spec.samples;
}

static int pan_sdl_get_sample_rate(pan_driver_t *zelf)
{
    pan_sdl_driver_t *self = (pan_sdl_driver_t *)zelf;
    return self->spec.freq;
}

static double pan_sdl_get_time(pan_driver_t *zelf)
{
    pan_sdl_driver_t *self = (pan_sdl_driver_t *)zelf;
    return self->time;
}

static int pan_sdl_configure_demo(pan_driver_t *zelf, demo_t *demo)
{
    pan_sdl_driver_t *self = (pan_sdl_driver_t *)zelf;

    if (!self->core_configure_demo(zelf, demo)) {
        return 0;
    }

    self->effect = demo->pan_effect_root;

    return 1;
}

pan_sdl_driver_t *pan_sdl_driver_instantiate(pan_sdl_driver_t *x)
{
    pan_sdl_driver_t *self = pan_sdl_driver_instantiate_super(x);
    pan_driver_t *interface = &self->super;

    interface->new = pan_sdl_new;
    interface->start = pan_sdl_start;
    interface->stop = pan_sdl_stop;
    interface->destroy = pan_sdl_destroy;
    interface->get_samples_number = pan_sdl_get_samples_number;
    interface->get_sample_rate = pan_sdl_get_sample_rate;
    interface->get_time = pan_sdl_get_time;

    self->core_configure_demo = interface->configure_demo;
    interface->configure_demo = pan_sdl_configure_demo;

    return self;
}

static void pan_sdl_driver_initialize() __attribute__((constructor));

static void pan_sdl_driver_initialize()
{
    put_pan_driver("sdl", &pan_sdl_driver_instantiate_toplevel(NULL)->super);
}
