/* a10 562
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/modplug/modplug_player.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 562 */

#ifndef _MODPLUG_PLAYER_H_
  #define _MODPLUG_PLAYER_H_

#include <libc/stdio.h>
#include <libc/stdint.h>
#include <third-party/libmodplug/modplug.h>
#include <library/stream.h>
#include <audio/resource_audio_effect.h>

typedef struct modplug_player_t 
{
    resource_audio_effect_t super;

    void (*set_row_callback)(struct modplug_player_t* self, row_callback_f cb, void* closure);
    void (*get_music_speed)(struct modplug_player_t* self, 
			    double* tick_duration, int* ticks_per_row);

    ModPlugFile* mf;
    ModPlug_Settings settings;

    int16_t* decoding_buffer;  /* modplug data */

    int running_p;
    float master_volume;
} modplug_player_t;

CLASS_INHERIT(modplug_player, resource_audio_effect)

#endif
