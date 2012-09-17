/* a10 974
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/text-display.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 974 */



#ifndef KNOS_DEMOS_1_1_TEXT_DISPLAY_H
#define KNOS_DEMOS_1_1_TEXT_DISPLAY_H

#include <system/effects.h>
#include <library/strings.h>
#include <freetype/ft_renderer.h>

typedef struct text_display_t
{
    video_effect_t super;

    void (*set_text)(struct text_display_t* self, string_t* string);
    /* switch to next line and display it between start and end */
    void (*next_line)(struct text_display_t* self, double start_ms, double end_ms);

    ft_renderer_t          ft;
    substring_generator_t* line_generator;
    string_t*              current_line;
    double                 start_ms;
    double                 end_ms;

    /* start x and y */
    double x, y;            
    unsigned int font_height;
} text_display_t;

CLASS_INHERIT(text_display, video_effect)

#endif
