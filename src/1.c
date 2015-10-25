/* a10 471
 * Copyright (c) 2001-2012 Nicolas LÃ©veillÃ© <knos.free.fr>
 *
 * You should have received this file ('src/1.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 471 */

/*
  1-1
*/

#ifdef MACOSX
/* this is needed so that macosx initialization code goes before
   our main.
   will have to be done in system.a
*/
#include <SDL.h>
#endif

#include <log4c.h>

LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_1_1);

#if KNOS_BUILD == KNOS_RELEASE
static char *module_url = "zip://virtual://resources/+1-1.dat#nsremix.it";
static char *frame_url = "zip://virtual://resources/+1-1.dat#frame.png";
static char *font_url =
    "zip://virtual://resources/+1-1.dat#RopaSans-Regular.otf"
#else
static char *module_url = "virtual://resources/dat/nsremix.it";
static char *frame_url = "virtual://resources/dat/frame.png";
static char *font_url = "virtual://resources/dat/RopaSans-Regular.otf";
#endif

    // network enabled by default
    static int network_p = 1;

#include <system/demo.h>
#include <system/effect.h>
#include <system/main.h>
#include <system/effects.h>
#include <lib/chance.h>

#include <lib/pixel.h>
#include <lib/url_open.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/math.h>

#include <modplug/modplug_player.h>

#include <messaging/receiver.h>
#include <scripting/dictionary.h>
#include <scripting/compile.h>

#include "alogic.h"
#include "projector.h"
#include "image.h"
#include "icefx.h"
#include "stutter.h"
#include "tap.h"
#include "vlogic.h"
#include "metro.h"
#include "blur.h"
#include "rub.h"
#include "vmix.h"
#include "gscreen.h"
#include "vloo.h"
#include "text-display.h"

effect_t *vlogic;
effect_t *tv;
#include "traffic.h"
#include <libc/pthread.h>

effect_t *projector;
pthread_mutex_t projector_mutex = PTHREAD_MUTEX_INITIALIZER;
#define IMAGES_N 37
static int images[IMAGES_N];
static int current_image;
static int next_image_n = 0;

int add_to_projector(stream_t *stream)
{
    int i = 0;

    pthread_mutex_lock(&projector_mutex);
#ifndef NDEBUG
    if (next_image_n >= IMAGES_N) {
        ERROR1("PRECONDITION: trying to add more images than we can hold!");
        return 0;
    }
#endif
    i = images[next_image_n] =
        ((projector_t *)projector)->add_image((projector_t *)projector, stream);
    if (i > 0)
        next_image_n++;
    pthread_mutex_unlock(&projector_mutex);

    return i;
}

effect_t *gscreen;
vmix_t *vmix;

effect_t *icefx;
double ice_ms = -1000.0;
atom_t ice_atom = 0;
atom_t alpha_atom = 0;

effect_t *vloo;

effect_t *rub;
/* some trigger */
double rub_ms = -10000.0;
unsigned int rub_length = 32;
atom_t rub_atom = 0;

effect_t *blur;
/* some trigger */
double blur_ms = -10000.0;
unsigned int blur_length = 32;

double zoom_ms = -10000.0;

text_display_t *text_display;

metronome_t *metro;
receiver_t *receiver;

/* audio */
audio_silence_t *silencer;
stutter_t *stutter;

modplug_player_t *modplug;
alogic_t *alogic;
metronome_t *metro2;

tapin_t *tapin;
tapout_t *tapout;
sample_t *buffer = NULL;

sample_t global_amp = 1.0f;

static void rub_callback(void *self, int length, context_t *context)
{
    if (context)
        rub_ms = context->ms;
    rub_length = length;

    if (length <= 1) {
        int i = 0;
        if (next_image_n == 0)
            return;

        do {
            int temp;

            pthread_mutex_lock(&projector_mutex);
            temp = unirand() * next_image_n;
            if (temp < next_image_n)
                current_image = temp;
            else
                current_image = next_image_n - 1;
            pthread_mutex_unlock(&projector_mutex);

            if (!images[current_image]) {
                i++;
            }
        } while (i && i < IMAGES_N);

        if (context)
            zoom_ms = context->ms;
    }
}

static void ice_callback(void *self, context_t *context)
{
    if (context)
        ice_ms = context->ms;
    else
        DEBUG1(__FILE__ ": no context");
}

static void ice_alpha_callback(icefx_t *self, unsigned int alpha,
                               context_t *context)
{
    self->alpha = alpha;
}

/*--- ui event processing ---*/

#include <system/event_listener.h>

static void switch_stutter(event_listener_t *self, const event_t *e)
{
    const key_event_t *ke = e;
    dictionary_t *dict = dictionary_get_instance();
    demo_t *demo = demo_get_instance();

    if (ke->hierarchy[1].verb == dict->get_atom(dict, "down")) {
        demo->send_immediate(demo, "integer:1 set-mode * route stutter route");
        rub->send_immediate(rub, "integer:1 set-mode");
        blur->send_immediate(blur, "integer:1 set-mode");
    } else if (ke->hierarchy[1].verb == dict->get_atom(dict, "up")) {
        demo->send_immediate(demo, "integer:0 set-mode * route stutter route");
        rub->send_immediate(rub, "integer:0 set-mode");
        blur->send_immediate(blur, "integer:0 set-mode");
    }
}

#include <widgets/gauge.h>

typedef struct stutter_control_t {
    event_listener_t super;

    int pressed_p;
    enum { SC_YNONE, SC_HIGH_BASE, SC_LOW_BASE, SC_FB } controlled_variableX;
    enum { SC_XNONE, SC_REPEAT_PROBA, SC_SPEED } controlled_variableY;

    int acquire_origin_p;
    int x0, y0;
    double adjustmentX;
    double adjustmentY;

    vgauge_t *high_gauge;
    vgauge_t *repeat_gauge;
    vgauge_t *low_gauge;
    vgauge_t *fb_gauge;
} stutter_control_t;

CLASS_INHERIT(stutter_control, event_listener)

stutter_control_t *stutter_ui;
static double low_base = 1.0 / 60.0;
static double high_base = 1.5;
static double high_limit = 0.0;
static float fb = 0.38f; /* feedback of delay */
static double repeat_proba = 0.0;
static int repeat_proba_just_changed_p = 0;
static double speed_base = 1.0;

static void stutter_control_setY(stutter_control_t *self, double adjustment)
{
    double delta = adjustment - self->adjustmentY;

    switch (self->controlled_variableY) {
    case SC_LOW_BASE:
        low_base += delta;
        /* when adjusting low_base, must adjust high_limit and
           clip high_base */
        high_limit -= delta;
        if (high_limit < 0.0)
            high_limit = 0.0;
        if (high_base > high_limit) {
            high_base = high_limit;
            self->high_gauge->set_percent(self->high_gauge,
                                          high_base / high_limit);
            TRACE3("high_base: %f (%f)", high_base, high_base / high_limit);
        }
        if (low_base > high_base)
            low_base = high_base;
        if (low_base < 0.0)
            low_base = 0.0;

        self->low_gauge->set_percent(self->low_gauge, low_base / high_base);
        TRACE3("low_base: %f (%f)", low_base, low_base / high_base);
        break;
    case SC_HIGH_BASE:
        high_base += delta;
        if (high_base < low_base)
            high_base = low_base;
        if (high_base > high_limit)
            high_base = high_limit;
        self->high_gauge->set_percent(self->high_gauge, high_base / high_limit);
        self->low_gauge->set_percent(self->low_gauge, low_base / high_base);
        TRACE3("high_base: %f (%f)", high_base, high_base / high_limit);
        break;
    case SC_FB:
        fb += delta;
        if (fb < 0.0f)
            fb = 0.0f;
        else if (fb > 1.0f)
            fb = 1.0f;
        self->fb_gauge->set_percent(self->fb_gauge, fb);
        TRACE2("fb: %f", fb);
        break;
    default:
        // nothing
        break;
    }
    self->adjustmentY = adjustment;
}

static void stutter_control_setX(stutter_control_t *self, double adjustment)
{
    double delta = adjustment - self->adjustmentX;

    switch (self->controlled_variableX) {
    case SC_REPEAT_PROBA:
        repeat_proba += delta;
        if (repeat_proba > 1.0)
            repeat_proba = 1.0;
        else if (repeat_proba < 0.0)
            repeat_proba = 0.0;
        self->repeat_gauge->set_percent(self->repeat_gauge, repeat_proba);
        repeat_proba_just_changed_p = 1;
        TRACE2("repeat proba: %f%%", 100.0 * repeat_proba);
        break;
    case SC_SPEED:
        if (speed_base >= 0.0)
            speed_base += delta;
        else
            speed_base -= delta;
        TRACE2("speed: %f%%", speed_base);
        break;
    default:
        // nothing
        break;
    }
    self->adjustmentX = adjustment;
}

static void stutter_control_accept(event_listener_t *zelf, const event_t *e)
{
    stutter_control_t *self = (stutter_control_t *)zelf;
    const mouse_event_t *event = e;
    dictionary_t *dict = dictionary_get_instance();
    /* mouse button or mouse motion */
    if (event->hierarchy[1].verb == dict->get_atom(dict, "button")) {
        /* up or down */
        if (event->hierarchy[2].verb == dict->get_atom(dict, "up")) {
            self->pressed_p = 0;
            self->controlled_variableX = SC_XNONE;
            self->controlled_variableY = SC_YNONE;
        } else if (!self->pressed_p &&
                   event->hierarchy[2].verb == dict->get_atom(dict, "down")) {
            self->pressed_p = 1;
            self->acquire_origin_p = 1;
            if (event->hierarchy[3].verb == dict->get_atom(dict, "lmb")) {
                self->controlled_variableY = SC_LOW_BASE;
                self->controlled_variableX = SC_REPEAT_PROBA;
            } else if (event->hierarchy[3].verb ==
                       dict->get_atom(dict, "rmb")) {
                self->controlled_variableY = SC_HIGH_BASE;
                self->controlled_variableX = SC_SPEED;
            } else if (event->hierarchy[3].verb ==
                       dict->get_atom(dict, "mmb")) {
                self->controlled_variableY = SC_FB;
            }
        }
    } else if (event->hierarchy[1].verb == dict->get_atom(dict, "move")) {
        if (self->pressed_p) {
            if (self->acquire_origin_p) {
                self->x0 = atom_get_integer_value(event->hierarchy[2].verb);
                self->y0 = atom_get_integer_value(event->hierarchy[3].verb);
                self->adjustmentX = 0.0;
                self->adjustmentY = 0.0;
                self->acquire_origin_p = 0;
            } else {
                double variation_y =
                    1.0 * atom_get_integer_value(event->hierarchy[3].verb) -
                    self->y0;
                double variation_x =
                    1.0 * atom_get_integer_value(event->hierarchy[2].verb) -
                    self->x0;
                variation_y /= demo_get_instance()->video_height;
                variation_x /= demo_get_instance()->video_width;
                stutter_control_setY(self, -variation_y);
                stutter_control_setX(self, variation_x);
            }
        }
    }
}

stutter_control_t *stutter_control_instantiate(stutter_control_t *x)
{
    stutter_control_t *sc = stutter_control_instantiate_super(x);

    sc->high_gauge = vgauge_instantiate_toplevel(NULL);
    sc->low_gauge = vgauge_instantiate_toplevel(NULL);
    sc->repeat_gauge = vgauge_instantiate_toplevel(NULL);
    sc->fb_gauge = vgauge_instantiate_toplevel(NULL);

    sc->super.accept = stutter_control_accept;

    return sc;
}

static void event_listeners_setup()
{
    dictionary_t *dict = dictionary_get_instance();

    /* filter space key events */
    {
        filtered_listener_t *fl_space_key;

        fl_space_key = malloc(sizeof(filtered_listener_t));
        /* initialize signature */
        signature_instantiate_toplevel(&fl_space_key->signature);
        key_event_instantiate(&fl_space_key->signature.super);

        /* both up and down space key */
        fl_space_key->signature.super.hierarchy[1].adverb =
            dict->get_atom(dict, "atom");
        fl_space_key->signature.super.hierarchy[1].verb =
            dict->get_atom(dict, "?");
        fl_space_key->signature.super.hierarchy[2].adverb =
            dict->get_atom(dict, "atom");
        fl_space_key->signature.super.hierarchy[2].verb =
            dict->get_atom(dict, "?");
        fl_space_key->signature.super.hierarchy[3].adverb =
            dict->get_atom(dict, "integer");
        fl_space_key->signature.super.hierarchy[3].verb = atom_new_integer(' ');

        /* initialize listener */
        fl_space_key->listener = event_listener_instantiate_toplevel(NULL);
        fl_space_key->listener->accept = switch_stutter;

        demo_get_instance()->ptc_listener->add_child(
            demo_get_instance()->ptc_listener, fl_space_key);
    }

    {
        stutter_control_t *sc = stutter_control_instantiate_toplevel(NULL);

        /* mouse button control */
        {
            filtered_listener_t *fl_mouse_button =
                malloc(sizeof(filtered_listener_t));
            signature_instantiate_toplevel(&fl_mouse_button->signature);

            /* we want both buttons, up and down */
            {
                signature_t *s = &fl_mouse_button->signature;

                mouse_button_event_instantiate(&s->super);
                s->super.hierarchy[2].adverb = dict->get_atom(dict, "atom");
                s->super.hierarchy[2].verb = dict->get_atom(dict, "?");
                s->super.hierarchy[3].adverb = dict->get_atom(dict, "atom");
                s->super.hierarchy[3].verb = dict->get_atom(dict, "?");
            }

            fl_mouse_button->listener = &sc->super;
            demo_get_instance()->ptc_listener->add_child(
                demo_get_instance()->ptc_listener, fl_mouse_button);
        }

        /* mouse motion control */
        {
            filtered_listener_t *fl_mouse_motion =
                malloc(sizeof(filtered_listener_t));
            signature_instantiate_toplevel(&fl_mouse_motion->signature);
            {
                signature_t *s = &fl_mouse_motion->signature;

                mouse_move_event_instantiate(&s->super);
                s->super.hierarchy[2].adverb = dict->get_atom(dict, "atom");
                s->super.hierarchy[2].verb = dict->get_atom(dict, "?");
                s->super.hierarchy[3].adverb = dict->get_atom(dict, "atom");
                s->super.hierarchy[3].verb = dict->get_atom(dict, "?");
            }
            fl_mouse_motion->listener = &sc->super;
            demo_get_instance()->ptc_listener->add_child(
                demo_get_instance()->ptc_listener, fl_mouse_motion);
        }

        stutter_ui = sc;
    }
}

void ajust_amp(sample_t amp) { global_amp *= 1.0 / amp; }

static void receiver_setup()
{
    dictionary_t *dict = dictionary_get_instance();
    rub_atom = dict->new_atom(dict, "rub");
    ice_atom = dict->new_atom(dict, "ice");
    alpha_atom = dict->new_atom(dict, "alpha");

    receiver = receiver_instantiate_toplevel(NULL);
    receiver->set_definition(receiver, rub_atom,
                             compile_cstring("integer", NULL),
                             (recp_f)rub_callback);
    receiver->close(receiver, rub_atom);
    receiver->set_definition(receiver, ice_atom, compile_cstring("", NULL),
                             (recp_f)ice_callback);
    receiver->set_definition(receiver, alpha_atom,
                             compile_cstring("integer", NULL),
                             (recp_f)ice_alpha_callback);
}

static void modplug_callback(void *self, double ms, unsigned int chn,
                             unsigned char note, unsigned char instr,
                             unsigned char volcmd, unsigned char volume,
                             unsigned char row_command, unsigned char row_param)
{
    const double diffms =
        ms + stutter->super.super.get_latency_ms(&stutter->super.super);
    if (instr == 1 || instr == 2 || instr == 3 || instr == 4) {
        context_t *c = context_instantiate_toplevel(NULL);
        bytecode_stream_t *stream = compile_cstring("ice", NULL);

        c->object = NULL;
        c->ms = diffms;
        receiver->receive(receiver, stream, c);
    } else {
        static int _11_fired_p[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bytecode_stream_t *stream = bytecode_stream_instantiate_toplevel(NULL);
        context_t *c = context_instantiate_toplevel(NULL);

        if (instr == 11) {
            unsigned int alpha = 256 - 256 * volume / 64;
            stream->append_integer(stream, alpha);
            stream->append_atom(stream, alpha_atom);
            c->object = icefx;
            c->ms = diffms;
            receiver->receive(receiver, stream, c);

            _11_fired_p[chn] = 1;
        } else if (_11_fired_p[chn]) {
            unsigned int alpha = 256;
            stream->append_integer(stream, alpha);
            stream->append_atom(stream, alpha_atom);
            c->object = icefx;
            c->ms = diffms;
            receiver->receive(receiver, stream, c);

            _11_fired_p[chn] = 0;
        }
    }
}

string_t *seer_string;
const char *seer_cstring_en =
    /* 1 */
    "I say that one must be a |seer|, make oneself a |seer|.\n"
    /* 2 */
    "The Poet makes himself a |seer| by a long, immense, and rational "
    "|derangement| of |all the senses|.\n"
    /* 3 */
    "All the forms of love, of suffering, of madness ;\n"
    /* 4 */
    "he searches himself, he consumes all the poisons in him, "
    "to only keep their quintessences.\n"
    /* 5 */
    "...\n"
    "\n"
    "\n"
    "\n"
    "\n"
    "\n"
    "\n"
    "...\n"
    /* 6 */
    "This future will be materialist, as you see ;\n"
    /* 7 */
    "- Always full of Number and Harmony, these poems will be made to stay.\n"
    /* 8 */
    "Eternal art would have its functions, since the poets are citizens.\n"
    /* 9 */
    "Poetry will not lend its rhythm to action ;\n"
    /* a */
    "it will |be ahead of it|.\n"
    "\n"
    /* b */
    "let us ask the |poets| for the |new| ; \n"
    /* c */
    "- ideas and forms.\n"
    /* d */
    "All the clever ones will soon believe they have "
    "satisfied this demand : \n"
    /* e */
    " - It is not so !\n"
    /* f */
    "inventions of the unknown demand new forms.\n";

const char *seer_cstring_fr =
    /* 1 */
    "Je dis qu'il faut être |voyant|, se faire |voyant|.\n"
    /* 2 */
    "Le Poète se fait |voyant| par un long, immense et raisonné "
    "|dérèglement| de |tous les sens|.\n"
    /* 3 */
    "Toutes les formes d'amour, de souffrance, de folie;\n"
    /* 4 */
    "il cherche lui-même, il épuise en lui tous les poisons, "
    "pour n'en garder que les quintessences.\n"
    /* 5 */
    "...\n"
    "\n"
    "\n"
    "\n"
    "\n"
    "\n"
    "\n"
    "...\n"
    /* 6 */
    "Cet avenir sera matérialiste, vous le voyez ;\n"
    /* 7 */
    "-- Toujours pleins du Nombre et de l'Harmonie "
    "ces poèmes seront faits pour rester.\n"
    /* 8 */
    "L'art éternel aurait ses fonctions ; comme les poètes "
    "sont citoyens.\n"
    /* 9 */
    "La Poésie ne rythmera plus l'action ;\n"
    /* a */
    "elle |sera en avant.|\n"
    "\n"
    /* b */
    "En attendant, demandons aux |poètes| du |nouveau| ;\n"
    /* c */
    " -- idées et formes.\n"
    /* d */
    "Tous les habiles croiraient bientôt avoir satisfait à cette demande.\n"
    /* e */
    " -- Ce n'est pas cela !\n"
    /* f */
    "Les inventions d'inconnu réclament des formes nouvelles.\n";

const char *seer_cstring_fi =
    /* 1 */
    "Jonkun täytyy olla |näkijä|, tee siis itsestäsi |näkijä|.\n"
    /* 2 */
    "Pitkä, suunnaton ja järjellinen |kaikkien aistien|"
    "|epäjärjestys| tekee runoilija |näkijän|.\n"
    /* 3 */
    "Kaikki rakkauden, kärsimyksen ja mielipuolisuuden muodot ;\n"
    /* 4 */
    "hän etsii itseään, käyttää ruumiin myrkkyjään, vain säilyttääkseen"
    "osan olennaista.\n"
    /* 5 */
    "..."
    "\n"
    "\n"
    "\n"
    "\n"
    "\n"
    "\n"
    "...\n"
    /* 6 */
    "Tulevaisuus on materialistinen, kuten voit nähdä ;\n"
    /* 7 */
    "Aina täynnä Lukuja ja Harmoniaa, runot tehtyinä jäädäkseen.\n"
    /* 8 */
    "Ikuinen taide täyttää tarkoituksensa, koska runoilijat ovat kansalaisia.\n"
    /* 9 */
    "Runous ei lainaa rytmiään tekoihin ;\n"
    /* 10 */
    "se on |edellä niitä|.\n"
    /* 11 */
    "Kysykäämme |runoilijalta| jotakin |uutta| ;\n"
    /* 12 */
    "- ajatuksia ja muotoa.\n"
    /* 13 */
    "Kaikki taitajat uskovat pian saavuttaneensa tämän vaatimuksen :\n"
    /* 14 */
    "- Näin ei tapahdu !\n"
    /* 15 */
    "tuntemattoman keksiminen vaatii uusia muotoja.\n";

#include <lib/locale.h>

static char *locale;

int bidule_new(effect_t *self)
{
    event_listeners_setup();

    if (network_p) {
        start_grabbing_images(IMAGES_N);
    }

    text_display->ft.use_font(&text_display->ft,
                              text_display->ft.load_font(
                                  &text_display->ft, url_open(font_url, "rb")));
    seer_string = string_instantiate_toplevel(NULL);

    /* choose text version */
    {
        TRACE2("locale: '%s'", locale);
        if (strstr(locale, "fr"))
            seer_string->new (seer_string, seer_cstring_fr);
        else if (strstr(locale, "fi"))
            seer_string->new (seer_string, seer_cstring_fi);
        else
            seer_string->new (seer_string, seer_cstring_en);
    }

    text_display->set_text(text_display, seer_string);

    if (stutter_ui) {
        stutter_ui->high_gauge->x = 24;
        stutter_ui->high_gauge->y = 64;
        stutter_ui->high_gauge->w = 3;
        stutter_ui->high_gauge->h = 24;

        stutter_ui->low_gauge->x = 24;
        stutter_ui->low_gauge->y = 96;
        stutter_ui->low_gauge->w = 3;
        stutter_ui->low_gauge->h = 24;

        stutter_ui->repeat_gauge->x = 32;
        stutter_ui->repeat_gauge->y = 96;
        stutter_ui->repeat_gauge->w = 3;
        stutter_ui->repeat_gauge->h = 24;

        stutter_ui->fb_gauge->x = 24;
        stutter_ui->fb_gauge->y = 128;
        stutter_ui->fb_gauge->w = 3;
        stutter_ui->fb_gauge->h = 24;
    }

    return 1;
}

void bidule_set_frame_size(video_effect_t *self, int width, int height,
                           int pitch)
{
    self->width = width;
    self->height = height;
    self->pitch = pitch;

    {
        video_effect_t *v = (video_effect_t *)vlogic;
        v->set_frame_size(v, self->width, self->height, self->pitch);

        v = (video_effect_t *)tv;
        v->set_frame_size(v, self->width, self->height, self->pitch);

        v = (video_effect_t *)projector;
        v->set_frame_size(v, self->width, self->height, self->pitch);

        v = (video_effect_t *)gscreen;
        v->set_frame_size(v, self->width, self->height, self->pitch);

        v = (video_effect_t *)icefx;
        v->set_frame_size(v, self->width, self->height, self->pitch);

        v = (video_effect_t *)vloo;
        v->set_frame_size(v, self->width, self->height, self->pitch);

        v = (video_effect_t *)rub;
        v->set_frame_size(v, self->width, self->height, self->pitch);

        v = (video_effect_t *)blur;
        v->set_frame_size(v, self->width, self->height, self->pitch);

        v = (video_effect_t *)vmix;
        v->set_frame_size(v, self->width, self->height, self->pitch);

        v = (video_effect_t *)text_display;
        v->set_frame_size(v, self->width, self->height, self->pitch);
    }

    ((projector_t *)tv)
        ->set_normalized_magnification((projector_t *)tv, 1, 1.0f);
}

uint32_t black_palette(unsigned int i, unsigned int max)
{
    unsigned char pof = 255L * i / max;
    return grey(pof);
}

static double fade_in_ms = 0.0;

uint32_t fade_in_palette(unsigned int i, unsigned int max)
{
    double l = fade_in_ms / 8000.0 * 255;
    double pof = l - l * i / max;
    unsigned char rpof = pof;
    return grey(rpof);
}

void bidule_compute(effect_t *e, void *content, double ms)
{
    static int flag_rub = 1;
    static int flag_pal = 1;
    double pfq = 60.0 / 126.0;

    {
        int i;

        modplug->get_music_speed(modplug, &pfq, &i);
        pfq *= i * 64;
        if (pfq == 0.0)
            pfq = 1.0 / 3000.0;
        else
            pfq = 1.0 / pfq;
    }

    if (ms < 80000.0) {
        if (ms < 8000.0) {
            fade_in_ms = ms;
            ((vlogic_t *)vlogic)
                ->generate_palette((vlogic_t *)vlogic, fade_in_palette);
        }
        ((vlogic_t *)vlogic)
            ->set_multiplier((vlogic_t *)vlogic, 1,
                             1.0 / (9.0 + 8.0 * sin(pfq * ms)));
    } else
        ((vlogic_t *)vlogic)
            ->set_multiplier((vlogic_t *)vlogic, 1,
                             1.0 / ceil(9.0 + 8.0 * sin(pfq * ms)));

    if (ms < 80000.0) {
        vlogic->computes(vlogic, content, ms);
    } else if (ms >= 80000.0 && ms < 92000.0) {
        vlogic->computes(vlogic, content, ms);
        gscreen->computes(gscreen, content, ms);
    } else if (ms >= 92000.0 && ms < 94000.0) {
        ((effect_t *)vmix)->computes((effect_t *)vmix, content, ms);
        vmix->set_mix(vmix, (94000.0 - ms) / 2000.0);
    } else if (ms >= 94000.0 && ms < 120000.0) {
        vloo->computes(vloo, content, ms);
    } else if (ms >= 120000.0) {
        if (flag_pal) {
            ((vlogic_t *)vlogic)
                ->generate_palette((vlogic_t *)vlogic, black_palette);
            flag_pal = 0;
        }

        vlogic->computes(vlogic, content, ms);
    }

    /* text */
    {
        static int occured_p = 0;

        metro_event_t *e =
            ((beatsource_t *)metro)->pump((beatsource_t *)metro, ms);

        if (e) {
            text_display->next_line(text_display, ms, ms + 10000.0);
            occured_p = 1;
        }

        if (occured_p)
            ((effect_t *)text_display)
                ->computes((effect_t *)text_display, content, ms);
    }

    /* projector, rub/blur, tv */
    {
        context_t c;

        context_instantiate_toplevel(&c);
        c.object = e;
        c.ms = ms;
        receiver->eval_pending(receiver, ice_atom, &c);

        if (ice_ms >= 0.0 &&
            fabs(ms - ice_ms) <= 2 * demo_get_instance()->video_frame_ms) {
            ((icefx_t *)icefx)->throw((icefx_t *)icefx, ms);
            ice_ms = -100.0;
        }

        c.object = icefx;
        receiver->eval_pending(receiver, alpha_atom, &c);
        context_retire(&c);
    }

    if (ms >= 93500.0) {
        context_t c;
        int rub_p = 0;
        int blur_p = 0;
        const int i_id = images[current_image];

        if (flag_rub) {
            receiver->open(receiver, rub_atom);
            flag_rub = 0;
        }

        context_instantiate_toplevel(&c);
        c.object = e;
        c.ms = ms;

        receiver->eval_pending(receiver, rub_atom, &c);
        context_retire(&c);

        if (rub_ms >= 0.0 && fabs(ms - rub_ms) <= 80.0) {
            ((blur_t *)rub)->set_length((blur_t *)rub, rub_length);
            blur_ms = ms;
            if (rub_length <= 1) {
                projector->set_mode(projector, ON);
                ((projector_t *)projector)
                    ->set_current((projector_t *)projector, i_id);
                blur_length = 8;
            } else {
                blur_length = rub_length;
                rub_p = 1;
            }
        } else if (blur_ms >= 0.0 &&
                   fabs(ms - blur_ms) <=
                       400.0 * ms * ms / (demo_get_instance()->end_ms *
                                          demo_get_instance()->end_ms)) {
            blur_p = 1;
            ((blur_t *)blur)->set_length((blur_t *)blur, blur_length);
        }

        if (ms >= zoom_ms && (ms - zoom_ms) <= 120.0) {
            float zoom = 1.0f + 0.55f * (ms - zoom_ms) / 120.0f;
            ((projector_t *)projector)
                ->set_normalized_magnification((projector_t *)projector, i_id,
                                               zoom);
        } else {
            projector->set_mode(projector, OFF);
        }

        projector->computes(projector, content, ms);
        icefx->computes(icefx, content, ms);
        if (rub_p && ms >= zoom_ms && (ms - zoom_ms) >= 80.0)
            rub->computes(rub, content, ms);
        if (blur_p)
            blur->computes(blur, content, ms);
    } else {
        icefx->computes(icefx, content, ms);
    }

    tv->computes(tv, content, ms);

    if (stutter_ui && stutter_ui->pressed_p) {
        video_effect_t *self = (video_effect_t *)e;
        image_t dest;
        dest.pixels = content;

        dest.width = self->width;
        dest.height = self->height;
        dest.pitch = self->pitch;

        stutter_ui->high_gauge->render(stutter_ui->high_gauge, &dest);
        stutter_ui->low_gauge->render(stutter_ui->low_gauge, &dest);
        stutter_ui->repeat_gauge->render(stutter_ui->repeat_gauge, &dest);
        stutter_ui->fb_gauge->render(stutter_ui->fb_gauge, &dest);
    }
}

video_effect_t *bidule_instantiate()
{
    video_effect_t *e = calloc(sizeof(video_effect_t), 1);

    video_effect_instantiate_toplevel(e);

    vlogic = (effect_t *)vlogic_instantiate_toplevel(NULL);
    vlogic->new (vlogic);

    tv = (effect_t *)projector_instantiate_toplevel(NULL);
    tv->new (tv);
    ((projector_t *)tv)
        ->add_image((projector_t *)tv, url_open(frame_url, "rb"));

    projector = (effect_t *)projector_instantiate_toplevel(NULL);
    projector->new (projector);
    projector->set_mode(projector, OFF);

    gscreen = (effect_t *)gscreen_instantiate_toplevel(NULL);
    gscreen->new (gscreen);

    vloo = (effect_t *)vloo_instantiate_toplevel(NULL);
    vloo->new (vloo);

    icefx = (effect_t *)icefx_instantiate_toplevel(NULL);
    icefx->new (icefx);

    rub = &rub_instantiate_toplevel(NULL)->super.super.super;
    rub->new (rub);
    ((blur_t *)rub)->set_length((blur_t *)rub, 32);

    blur = &blur_instantiate_toplevel(NULL)->super.super;
    blur->new (blur);
    ((blur_t *)blur)->set_length((blur_t *)blur, 32);

    vmix = vmix_instantiate_toplevel(NULL);
    ((effect_t *)vmix)->new ((effect_t *)vmix);
    vmix->set_left_effect(vmix, (video_effect_t *)vlogic);
    vmix->set_right_effect(vmix, (video_effect_t *)vloo);
    vmix->set_right_offset_ms(vmix, 92000.0);

    text_display = text_display_instantiate_toplevel(NULL);
    ((effect_t *)text_display)->new ((effect_t *)text_display);

    e->super.new = bidule_new;
    e->super.computes = bidule_compute;
    e->set_frame_size = bidule_set_frame_size;

    return e;
}

static void truc_set_area_parameters(audio_effect_t *a, int sample_rate,
                                     int frame_number, int frame_size)
{
    a->sample_rate = sample_rate;
    a->frame_number = frame_number;
    a->frame_size = frame_size;

    silencer->super.set_area_parameters(&silencer->super, sample_rate,
                                        frame_number, frame_size);
    modplug->super.super.set_area_parameters(&modplug->super.super, sample_rate,
                                             frame_number, frame_size);
    alogic->super.set_area_parameters(&alogic->super, sample_rate, frame_number,
                                      frame_size);
    stutter->super.set_area_parameters(&stutter->super, sample_rate,
                                       frame_number, frame_size);
    tapin->super.set_area_parameters(&tapin->super, sample_rate, frame_number,
                                     frame_size);
    tapout->super.set_area_parameters(&tapout->super, sample_rate, frame_number,
                                      frame_size);

    buffer = realloc(buffer, frame_number * frame_size * sizeof(sample_t));
    {
        unsigned int n = frame_number;
        sample_t *samples = buffer;
        while (n--) {
            samples[0] = 0.0f;
            samples[1] = 0.0f;
            samples += frame_size;
        }
    }

    {
        unsigned int samples;
        double p = 60.0 / 126.0;
        int i;

        modplug->get_music_speed(modplug, &p, &i);
        p *= i;

        samples = a->sample_rate * p / 1000.0 / 8.0;

        stutter->set_block_size(stutter, samples);
        stutter->set_speed(stutter, 1.0);
        stutter->set_window(stutter, NONE);

        {
            // set tapin size to a reasonable one (6sec)
            int size = 6.0 * a->sample_rate;
            size = size > a->frame_number ? size : a->frame_number;
            tapin->set_size(tapin, size);
        }

        {
            // set offset of tapout
            int offset = p * 3.0 / 1000.0 * a->sample_rate;
            tapout->set_tapin(tapout, tapin);
            tapout->set_offset(tapout, tapin->line_size - offset);
        }
    }
}

static void truc_computes_area(audio_effect_t *self, audio_area_t *area,
                               double ms)
{
    static double proba1 = 0.0;
    double proba2 = 0.0;
    static int flag1 = 1;
    static int flag2 = 1;
    static int flag3 = 1;
    static int flag4 = 1;

    if (flag1) {
        fb = 0.38f;
        if (stutter_ui)
            stutter_ui->fb_gauge->set_percent(stutter_ui->fb_gauge, fb);
        flag1 = 0;
    } else if (ms >= 80000.0 && flag2) {
        fb = 0.6f;
        if (stutter_ui)
            stutter_ui->fb_gauge->set_percent(stutter_ui->fb_gauge, fb);
        flag2 = 0;
    } else if (ms >= 93500 && flag3) {
        stutter->set_window(stutter, GAUSSIAN);
        flag3 = 0;
    } else if (ms >= 120000 && flag4) {
        stutter->set_window(stutter, NONE);
        fb = 0.38f;
        if (stutter_ui)
            stutter_ui->fb_gauge->set_percent(stutter_ui->fb_gauge, fb);
        flag4 = 0;
    }

    proba1 = 0.24 * (-cos(ms / 20000.0) + 1.0) / 2.0;
    proba2 =
        (sin(sin(ms / 20000.0)) + cos(cos(ms / 20000.0)) + sin(ms / 103000.0)) /
        3.0;

    {
        metro_event_t *e;

        e = ((beatsource_t *)metro2)->pump((beatsource_t *)metro2, ms);

        if (ms >= 80000.0) {
            if (e && unirand() < proba1) {
                speed_base *= -1.0;
            }
        }
        if (e && unirand() < proba2) {
            double p;
            int samples;
            static int flag_1_p = 1;
            static int flag_2_p = 1;

            if (flag_1_p) {
                /* initialization */
                low_base = 1.0 / 60.0;
                high_base = 1.5;
                if (stutter_ui) {
                    stutter_ui->low_gauge->set_percent(stutter_ui->low_gauge,
                                                       low_base);
                    stutter_ui->high_gauge->set_percent(stutter_ui->high_gauge,
                                                        high_base);
                }
                flag_1_p = 0;
            }

            if (ms >= 80000.0 && flag_2_p) {
                low_base = 1.0 / 180.0;
                high_base = 1.0;
                if (stutter_ui) {
                    stutter_ui->low_gauge->set_percent(stutter_ui->low_gauge,
                                                       low_base);
                    stutter_ui->high_gauge->set_percent(stutter_ui->high_gauge,
                                                        high_base);
                }
                flag_2_p = 0;
            }

            {
                int i;

                modplug->get_music_speed(modplug, &p, &i);
                p *= i;
            }

            if (p > 0)
                high_limit =
                    65536.0 * 1000.0 / (self->sample_rate * p) - low_base;

            samples = 1.0 * self->sample_rate / 1000.0 * p *
                      (low_base + high_base * exprand());

            stutter->set_block_size(stutter, samples);
            {
                context_t *c = context_instantiate_toplevel(NULL);
                bytecode_stream_t *stream =
                    bytecode_stream_instantiate_toplevel(NULL);

                stream->append_integer(stream,
                                       256 * stutter->block_size / 65536);
                stream->append_atom(stream, rub_atom);

                c->object = NULL;
                /* latency of stutter object is roughly equal to block_size */
                c->ms =
                    ms +
                    stutter->super.super.get_latency_ms(&stutter->super.super);
                receiver->receive(receiver, stream, c);
            }
        }
    }

    /* speed adjustment */
    {
        stutter->set_speed(stutter, speed_base + 0.2 * sin(ms / 10000.0));
    }

    /* repeat probability */
    {
        double sine = (1.0 - cos(ms / 11000.0)) * 0.485;
        static double delta = 0.0;

        if (repeat_proba_just_changed_p) {
            delta = repeat_proba - sine;
            repeat_proba_just_changed_p = 0;
        }

        repeat_proba = sine + delta;
        if (repeat_proba > 1.0)
            repeat_proba = 1.0;
        else if (repeat_proba < 0.0)
            repeat_proba = 0.0;
        if (stutter_ui)
            stutter_ui->repeat_gauge->set_percent(stutter_ui->repeat_gauge,
                                                  repeat_proba);

        stutter->set_repeat_probability(stutter, sine + delta);
    }
    stutter->set_dropout_probability(stutter, (1.0 - cos(ms / 9500.0)) * 0.05);

    alogic->super.super.computes((effect_t *)alogic, area, ms);
    modplug->super.super.super.computes((effect_t *)modplug, area, ms);

    stutter->super.super.computes((effect_t *)stutter, area, ms);
    {
        // mix output from delay line then feeds the result to the input
        int i;
        sample_t *samples = area->samples + area->head * self->frame_size;

        for (i = 0; i < area->frame_number; i++) {
            // fb: 0.58f
            samples[i * self->frame_size] +=
                fb * buffer[self->frame_size * i + 1];
            samples[i * self->frame_size + 1] +=
                fb * buffer[self->frame_size * i];
        }

        tapin->super.super.computes(&tapin->super.super, area, ms);

        for (i = 0; i < area->frame_number; i++) {
            samples[i * self->frame_size] *= 0.22f * global_amp;
            samples[i * self->frame_size + 1] *= 0.22f * global_amp;
        }
    }

    {
        audio_area_t zarea;
        zarea.head = 0;
        zarea.samples = buffer;
        zarea.frame_number = self->frame_number;

        tapout->super.super.computes(&tapout->super.super, &zarea, ms);
    }
}

audio_effect_t *truc_instantiate()
{
    audio_effect_t *a = audio_effect_instantiate_toplevel(NULL);

    modplug = modplug_player_instantiate_toplevel(NULL);
    ((effect_t *)modplug)->new ((effect_t *)modplug);

    if (!modplug->super.load_file(&modplug->super, url_open(module_url, "rb"),
                                  NULL)) {
        ERROR2("modplug could not load %s", module_url);
    }
    {
        context_t c;
        context_instantiate_toplevel(&c);
        c.ms = 0.;

        modplug->super.set_master_volume(&modplug->super, 10.0, &c);
        context_retire(&c);
    }
    modplug->set_row_callback(modplug, modplug_callback, NULL);

    {
        double p;
        int i;
        modplug->get_music_speed(modplug, &p, &i);
        p *= i;
        metro2->new (metro2, 0.0, p * 4, 50.0);
    }

    silencer = audio_silence_instantiate_toplevel(NULL);
    silencer->super.super.new(&silencer->super.super);
    silencer->set_next_effect(silencer, (audio_effect_t *)modplug);

    alogic = alogic_instantiate_toplevel(NULL);
    alogic->super.super.new(&alogic->super.super);

    stutter = stutter_instantiate_toplevel(NULL);
    stutter->super.super.new(&stutter->super.super);

    tapin = tapin_instantiate_toplevel(NULL);
    tapin->super.super.new(&tapin->super.super);

    tapout = tapout_instantiate_toplevel(NULL);
    tapout->super.super.new(&tapout->super.super);

    a->set_area_parameters = truc_set_area_parameters;
    a->computes_area = truc_computes_area;

    return a;
}

int main(int argc, char **argv)
{
    demo_t *demo;

    log_setControlString("KNOS_DEMOS_SYSTEM.thresh=2"
                         " KNOS_DEMOS_1_1.thresh=5"
                         " KNOS_DEMOS_1_1_IMAGE_LOAD_PNG.thresh=6"
                         " KNOS_NETWORK_SHARED_TRANSPORT.thresh=6"
                         " KNOS_LIBRARY_HTTP_STREAM.thresh=6"
                         " KNOS_DEMOS_1_1_TRAFFIC.thresh=6"
                         " KNOS_DEMOS_1_1_IMAGE_FETCHER.thresh=6"
                         //" ROUTER.thresh=0"
                         //" KNOS_SCRIPTING_DICTIONARY.thresh=0"
                         //" KNOS_MESSAGING_CLASS.thresh=0"
                         //" KNOS_SCRIPTING_COMPILE.thresh=0"
                         //" TINYPTC.thresh=0"
                         );

    demo = demo_get_instance();
    demo->title = "+1-1";
    demo->video_width = 800;
    demo->video_height = 600;
    demo->video_buffers = 6;
    demo->video_frame_ms = 20.0;
    demo->audio_sample_rate = 44100;
    demo->audio_device = "default";
    demo->audio_buffers = 4;
    demo->nosound = 0;
    demo->osc = 0;
    demo->end_ms = 380000.0;

    locale = strdup(get_locale());

    /* process command line */
    {
        int i;
        for (i = 1; i < argc; i++) {
            if (!strcmp(argv[i], "nosound")) {
                demo->nosound = 1;
            } else if (!strcmp(argv[i], "nonet")) {
                network_p = 0;
            } else if (!strncmp(argv[i], "audiodevice=", 12)) {
                demo->audio_device = strdup(argv[i] + 12);
            } else if (!strncmp(argv[i], "module=", 7)) {
                module_url = strdup(argv[i] + 7);
                /* an unknown track, thus we enable amplification adjustment */
                demo->pan_overflow_callback = ajust_amp;
            } else if (!strcmp(argv[i], "freeze")) {
                traffic_freeze_p(1);
            } else if (!strcmp(argv[i], "restore")) {
                traffic_restore_p(1, NULL);
            } else if (!strncmp(argv[i], "restore=", 8)) {
                traffic_restore_p(1, strdup(argv[i] + 8));
            } else if (!strncmp(argv[i], "locale=", 7)) {
                locale = strdup(argv[i] + 7);
            }
        }
    }

    receiver_setup();
    {
        double p = 10000.0;
        metro = metronome_instantiate_toplevel(NULL);
        metro->new (metro, 120000.0, p, 50.0);

        metro2 = metronome_instantiate_toplevel(NULL);
    }

    demo->set_video_effect(demo, &bidule_instantiate()->super);
    demo->set_audio_effect(demo, &truc_instantiate()->super);

    demo->running_p = 1;

    return main_loop(demo);
}
