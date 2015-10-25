/* a10 840
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/text-display.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 840 */

#include "text-display.h"
#include <lib/pixel.h>
#include <lib/chance.h>
#include <libc/stdlib.h>

static void text_display_set_text(text_display_t *self, string_t *string)
{
    if (string) {
        self->line_generator =
            string->get_substring_generator(string, '\n', NULL);
        self->current_line = NULL;
    }
}

static void text_display_next_line(text_display_t *self, double start_ms,
                                   double end_ms)
{
    if (self->line_generator) {
        if (self->current_line) {
            self->current_line->destroy(self->current_line);
            object_retire(string_to_object(self->current_line));
        }
        self->current_line = self->line_generator->next(self->line_generator);
        self->start_ms = start_ms;
        self->end_ms = end_ms;
    }
}

static void text_display_computes(effect_t *e, void *content, double ms)
{
    text_display_t *self = (text_display_t *)e;
    image_t dest;

    dest.pixels = content;
    dest.pitch = dest.width = self->super.width;
    dest.height = self->super.height;

    if (ms >= self->start_ms && ms <= self->end_ms && self->current_line) {
        substring_generator_t *word_generator;
        if ((word_generator = self->current_line->get_substring_generator(
                 self->current_line, ' ', NULL))) {
            string_t *word;
            int i = 0;
            static int selekt = -1;
            static double selekt_ms = -200.0;
            static unsigned int selekt_fh = 24;
            unsigned int alpha = 0x7a;

            if (ms <= self->start_ms + 320.0)
                alpha = (int)(alpha * (ms - self->start_ms) / 320.0) << 24;
            else if (ms >= self->end_ms - 320.0)
                alpha = (int)(alpha * (self->end_ms - ms) / 320.0) << 24;
            else
                alpha = alpha << 24;

            self->ft.pen_y = self->y;
            self->ft.color = alpha | (grey(11) & 0xffffff);
            self->ft.pen_x = self->super.width;
            while ((word = word_generator->next(word_generator))) {
                self->ft.font_height = self->font_height;

                if (self->ft.pen_x +
                        word->get_length(word) * self->font_height / 2 >
                    self->super.width - self->x) {
                    self->ft.pen_x = self->x;
                    self->ft.pen_y += self->ft.font_height * 0.55;
                }

                if (selekt < 0 && unirand() > 0.95) {
                    selekt = i;
                    selekt_ms = ms;
                    selekt_fh = self->font_height * (1.0 + 2.0 * exprand());
                }

                if (selekt == i && ms >= selekt_ms) {
                    if (ms >= selekt_ms + 100.0) {
                        selekt = -1;
                        selekt_ms = -100.0;
                    } else
                        self->ft.font_height =
                            self->font_height +
                            (selekt_fh - self->font_height) *
                                (1.0 - (ms - selekt_ms) / 100.0);
                }

                {
                    int x = self->ft.pen_x;
                    self->ft.draw_string(&self->ft, &dest, word);
                    self->ft.pen_x = x + self->ghost_offset;
                    self->ft.draw_string(&self->ft, &dest, word);
                    self->ft.pen_x += self->ft.font_height / 3;
                }

                i++;
                word->destroy(word);
                object_retire(string_to_object(word));
            }
            if (selekt >= i)
                selekt = -1;

            free(word_generator);
        }
    }
}

void text_display_set_frame_size(video_effect_t *self, int width, int height,
                                 int pitch)
{
    text_display_t *t = (text_display_t *)self;

    t->super.width = width;
    t->super.height = height;
    t->super.pitch = pitch;

    double scale = height / 240.0;
    int font_height = 24 * scale;

    t->font_height = font_height;
    t->x = font_height;
    t->y = font_height;
    t->ghost_offset = 3 * scale;
}

text_display_t *text_display_instantiate(text_display_t *x)
{
    text_display_t *t = text_display_instantiate_super(x);

    ft_renderer_instantiate_toplevel(&t->ft);
    t->ft.new(&t->ft);

    t->super.super.computes = text_display_computes;
    t->super.set_frame_size = text_display_set_frame_size;
    t->set_text = text_display_set_text;
    t->next_line = text_display_next_line;

    return t;
}
