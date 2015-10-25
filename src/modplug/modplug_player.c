/* a10 167
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/modplug/modplug_player.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 167 */

#include "modplug_player.h"

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_MODPLUG_MODPLUG_PLAYER);

#include <system/pan.h>
#include <libc/stdlib.h>
#include <audio/resource_audio_effect_impl.h>

static int modplug_player_new(effect_t *self)
{
    modplug_player_t *p = (modplug_player_t *)self;

    p->running_p = 0;
    p->master_volume = 1.0f;

    {
        context_t c;
        context_instantiate_toplevel(&c);
        c.ms = 0.;
        p->super.set_looping_mode(&p->super, 1, &c);
        context_retire(&c);
    }

    return 1;
}

static int modplug_player_destroy(effect_t *self)
{
    modplug_player_t *p = (modplug_player_t *)self;

    if (p->decoding_buffer)
        free(p->decoding_buffer);
    return 1;
}

static void modplug_player_set_area_parameters(audio_effect_t *self, int srate,
                                               int frames, int frame_size)
{
    modplug_player_t *player = (modplug_player_t *)self;

    ModPlug_GetSettings(&player->settings);
    switch (srate) {
    case 44100:
    case 22050:
    case 11025:
        player->settings.mFrequency = srate;
        break;
    default:
        ERROR1("srate unsupported by modplug.");
    }

    player->settings.mChannels = 2;
    player->settings.mBits = 16;
    player->settings.mResamplingMode =
        MODPLUG_RESAMPLE_LINEAR; // MODPLUG_RESAMPLE_FIR;
    player->settings.mFlags = MODPLUG_ENABLE_OVERSAMPLING;

    ModPlug_SetSettings(&player->settings);

    self->sample_rate = srate;
    self->frame_number = frames;
    self->frame_size = frame_size;

    if (player->decoding_buffer)
        a_free(player->decoding_buffer);
    player->decoding_buffer = a_calloc(frame_size * sizeof(sample_t), frames);
}

static void add_16toSample(sample_t *out, int16_t *in, unsigned long num,
                           float volume)
{
    double factor = 32767.0;
    int32_t smp;

    for (; num--; ++out, ++in) {
        smp = *in;
        smp = smp > 32767 ? 32767 : smp < -32768 ? -32768 : smp;

        *out += volume *(sample_t)((signed short)(smp) / factor);
    }
}

static void modplug_player_computes_area(audio_effect_t *self,
                                         audio_area_t *area, double ms)
{
    modplug_player_t *p = (modplug_player_t *)self;
    sample_t *__restrict__ samples =
        area->samples + area->head * p->super.super.frame_size;

    int len = p->super.super.frame_size * area->frame_number;
    int n = sizeof(int16_t) * len; /* in bytes, 16bit */

    if (!p->running_p) {
        /* outputs silence */
        int i;
        for (i = 0; i < len; i++)
            samples[i] = 0.0f;
    } else {
        int nread;
        char *buffer = (char *)p->decoding_buffer;
        do {
            nread = ModPlug_Read(p->mf, buffer, n);
            buffer += nread;
        } while (nread != 0 && (n -= nread) > 0);
        add_16toSample(samples, p->decoding_buffer, len, p->master_volume);
    }
}

static void modplug_player_eom(void *zelf, double ms)
{
    resource_audio_effect_t *self = zelf;
    TRACE2("sending eos (%f)", ms);
    resource_audio_effect_send_eos_message(self, ms);
}

static int modplug_player_load_stream(resource_audio_effect_t *zelf,
                                      stream_t *stream, progress_t *progress)
{
    modplug_player_t *self = (modplug_player_t *)zelf;
    int status = 1;
    int64_t n;
    long size;
    char *buffer;

    if (!stream)
        return 0;

    if (progress) {
        progress->set_state(progress, RUNNING);
    }

    ModPlug_GetSettings(&self->settings);
    self->settings.mChannels = 2;
    self->settings.mBits = 16;
    ModPlug_SetSettings(&self->settings);

    buffer = stream->get_as_memory_area(stream, &n);

    if (!buffer || !n)
        status = 0;

    size = n;

    if (progress) {
        maybe_size_t N;
        MAYBE_INITIALIZE(&N);
        MAYBE_SET_VALUE(&N, size);

        progress->set_max_n(progress, N);
    }

    /* file read in buffer */
    if (status) {
        self->mf = ModPlug_Load(buffer, size);
    }
    if (status && !self->mf)
        status = 0;
    stream->free_memory_area(stream, buffer, size);

    if (status) {
        self->running_p = 1;
        ModPlug_SetEndOfModuleCallback(self->mf, modplug_player_eom, zelf);
    }

    if (progress) {
        if (status)
            progress->set_state(progress, FINISHED);
        else
            progress->set_state(progress, ABORTED);
    }

    return status;
}

static void modplug_player_set_master_volume(resource_audio_effect_t *zelf,
                                             const float volume, context_t *c)
{
    modplug_player_t *self = (modplug_player_t *)zelf;
    if (self->running_p)
        self->master_volume = volume;
}

static void modplug_player_set_row_callback(modplug_player_t *self,
                                            row_callback_f cb, void *closure)
{
    if (self->running_p)
        ModPlug_SetRowCallback(self->mf, cb, closure);
}

static void modplug_player_set_looping_mode(resource_audio_effect_t *zelf,
                                            atom_t looping_p, context_t *c)
{
    modplug_player_t *self = (modplug_player_t *)zelf;

    self->settings.mLoopCount = looping_p ? -1 : 0;
    if (self->mf)
        ModPlug_SetRepeatCount(self->mf, looping_p ? -1 : 0);
}

static void modplug_player_get_music_speed(modplug_player_t *self,
                                           double *tick_duration,
                                           int *ticks_per_row)
{
    if (self->running_p) {
        unsigned int n;
        ModPlug_GetMusicSpeed(self->mf, tick_duration, &n);
        *ticks_per_row = (int)n;
    }
}

modplug_player_t *modplug_player_instantiate(modplug_player_t *x)
{
    modplug_player_t *p = modplug_player_instantiate_super(x);

    p->super.super.super.new = modplug_player_new;
    p->super.super.super.destroy = modplug_player_destroy;
    p->super.super.computes_area = modplug_player_computes_area;
    p->super.super.set_area_parameters = modplug_player_set_area_parameters;

    p->super.load_file = modplug_player_load_stream;
    p->super.set_master_volume = modplug_player_set_master_volume;
    p->super.set_looping_mode = modplug_player_set_looping_mode;

    //    p->get_total_time   = modplug_player_get_total_time;
    //    p->get_current_time = modplug_player_get_current_time;

    p->set_row_callback = modplug_player_set_row_callback;
    p->get_music_speed = modplug_player_get_music_speed;

    return p;
}
