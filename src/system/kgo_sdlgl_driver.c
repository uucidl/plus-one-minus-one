/* a10 764
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/kgo_sdlgl_driver.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 764 */

#include "kgo_driver.h"

#define NO_SDL_GLEXT
#include <SDL.h>
#include <SDL_opengl.h>

#include <scripting/dictionary.h>
#include <system/opengl_effect.h>
#include <system/opengl_adapter.h>

#include <logging.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_SYSTEM_KGO_SDLGL_DRIVER);

typedef struct kgo_sdlgl_driver_t {
    kgo_driver_t super;

    /* set up before calling new */
    int windowed_p;

    int width;
    int height;

    /* physical window size */
    int window_width;
    int window_height;

    int has_adapter_p;

    SDL_Window *window;
  
#ifdef LINUX
    Display *x11_display;
    Window x11_window;
#endif

    demo_t *demo;
} kgo_sdlgl_driver_t;

CLASS_INHERIT(kgo_sdlgl_driver, kgo_driver);

#ifdef WIN32

#include <SDL_syswm.h>
#include <windows.h>
#include <shellapi.h>

static void setupWIN32(kgo_sdlgl_driver_t *self)
{
    int r;
    SDL_SysWMinfo info;
    HWND hWnd;

    SDL_VERSION(&(info.version));
    r = SDL_GetWindowWMInfo(self->window, &info);
    if (r < 0) {
        ERROR("Error setting up WIN32 specifics: '%s'\n", SDL_GetError());
        return;
    }
    hWnd = info.window;

    DragAcceptFiles(hWnd, TRUE);

    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
}

#elif defined(LINUX)

#include <SDL_syswm.h>

static void setupLINUX(kgo_sdlgl_driver_t *self)
{
    int r;
    SDL_SysWMinfo info;

    SDL_VERSION(&(info.version));
    r = SDL_GetWindowWMInfo(self->window, &info);
    if (r < 0) {
        ERROR("Error getting Linux specifics: '%s'\n", SDL_GetError());
        return;
    }

    self->x11_display = info.info.x11.display;
    self->x11_window = info.info.x11.window;

    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
}

static Display *kgo_sdlgl_get_display(kgo_driver_t *zelf)
{
    kgo_sdlgl_driver_t *self = (kgo_sdlgl_driver_t *)zelf;

    return self->x11_display;
}

static Window kgo_sdlgl_get_window(kgo_driver_t *zelf)
{
    kgo_sdlgl_driver_t *self = (kgo_sdlgl_driver_t *)zelf;

    return self->x11_window;
}

#elif MACOSX

#include <system/architectures/cocoa/dnd.h>

static void setupMACOSX(kgo_sdlgl_driver_t *self)
{
    kgo_driver_t *zelf = (kgo_driver_t *)self;
    dnd_init(zelf->get_event_listener(zelf));
}

#endif

static int kgo_sdlgl_driver_new(kgo_driver_t *zelf, char *title,
                                unsigned int width, unsigned int height)
{
    kgo_sdlgl_driver_t *self = (kgo_sdlgl_driver_t *)zelf;

    /* opengl can scale, thus the real window size will probably
       come from the user.
    */
    self->window_width = width;
    self->window_height = height;
    
    if (self->window_width < 640) {
      self->window_width = 640;
      self->window_height = self->window_width * height / width;
    }
    
    self->width = width;
    self->height = height;
    
    int ok = SDL_Init(SDL_INIT_VIDEO) == 0;
    if (!ok) {
      return 0;
    }

    atexit(SDL_Quit);

    self->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, self->window_width, self->window_height, SDL_WINDOW_OPENGL | (self->windowed_p? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP));
    if (!self->window) {
      return 0;
    }

    SDL_GLContext glcontext = SDL_GL_CreateContext(self->window);

#ifdef WIN32
    setupWIN32(self);
#elif defined(LINUX)
    setupLINUX(self);
#elif defined(MACOSX)
    setupMACOSX(self);
#endif

    return 1;
}

extern void ptc_cleanup_callback(void);

static int kgo_sdlgl_driver_destroy(struct kgo_driver_t *self)
{
    SDL_ShowCursor(1);
    SDL_Quit();

    // @todo remove gl context here.
    //SDL_GL_DeleteContext(glcontext);      
    
    return 1;
}

static int kgo_sdlgl_driver_has_fd(struct kgo_driver_t *self)
{
#ifdef LINUX
    return 0;
#else
    return -1;
#endif
}

static int kgo_sdlgl_driver_get_fd(struct kgo_driver_t *self) { return -1; }

#include <system/architectures/sdl/events.h>

static sdl_event_listener_t *event_listener = NULL;

static event_listener_t *
kgo_sdlgl_driver_get_event_listener(struct kgo_driver_t *zelf)
{
    kgo_sdlgl_driver_t *self = (kgo_sdlgl_driver_t *)zelf;
    if (!event_listener) {
        event_listener = sdl_event_listener_instantiate_toplevel(NULL);
        if (self->windowed_p && (self->window_width != self->width ||
                                 self->window_height != self->height)) {
            event_listener->set_mouse_motion_correction(
                event_listener, 1. * self->width / self->window_width,
                1. * self->height / self->window_height);
        }
    }

    return &event_listener->super;
}

static int process_events(kgo_driver_t *zelf, int event_pending)
{
    kgo_sdlgl_driver_t *self = (kgo_sdlgl_driver_t *)zelf;
    SDL_Event sdl_event;
    unsigned char const *keypressed;

    while (SDL_PollEvent(&sdl_event)) {
        switch (sdl_event.type) {
        case SDL_KEYDOWN: {
            keypressed = SDL_GetKeyboardState(NULL);
            if (keypressed[SDLK_ESCAPE] == SDL_PRESSED) {
                self->demo->running_p = 0;
            }
        }; break;
        case SDL_QUIT: {
            self->demo->running_p = 0;
        }; break;
        }
        if (event_listener) {
            /* forward sdl event to sdl event listener */
            event_listener->accept(event_listener, &sdl_event);
        }
    }

    return 0;
}

static void kgo_sdlgl_driver_update(struct kgo_driver_t *zelf, void *buffer,
                                    int event_pending)
{
    kgo_sdlgl_driver_t *self = (kgo_sdlgl_driver_t *)zelf;

    if (buffer != NULL && self->has_adapter_p) {
        opengl_adapter_t *adapter =
            (opengl_adapter_t *)self->demo->kgo_effect_root;
        frame_converter_t *converter = adapter->fc;
        frame_converter_frame_t frame;
        frame.input_frame = buffer;
        frame.output_frame = NULL;
        converter->computes(converter, &frame, 0.0);
    }
    SDL_GL_SwapWindow(self->window);
    process_events(zelf, event_pending);
}

static atom_t kgo_sdlgl_driver_get_frame_type(struct kgo_driver_t *zelf)
{
    return dictionary_get_instance()->new_atom(dictionary_get_instance(),
                                               OPENGL_FRAME_TYPE_NAME);
}

static void *kgo_sdlgl_driver_allocate_frame(struct kgo_driver_t *zelf)
{
    return NULL;
}

static int kgo_sdlgl_driver_configure_demo(kgo_driver_t *zelf, demo_t *demo)
{
    kgo_sdlgl_driver_t *self = (kgo_sdlgl_driver_t *)zelf;
    self->demo = demo;

    int error_p = 0;

    demo->video_buffers = 2; // buffering is not supported with opengl
    demo->set_gui_event_listener(demo, zelf->get_event_listener(zelf));

    error_p = !demo->kgo_effect_root;

    if (!error_p) {
        atom_t frame_type =
            demo->kgo_effect_root->get_frame_type(demo->kgo_effect_root);
        dictionary_t *dict = dictionary_get_instance();

        if (frame_type == dict->get_atom(dict, VIDEO_EFFECT_FRAME_TYPE_NAME)) {
            /* adapt a video_effect_t to the driver */
            opengl_adapter_t *adapter =
                opengl_adapter_instantiate_toplevel(NULL);
            effect_t *effect = demo->kgo_effect_root;

            if (!adapter->super.super.new(&adapter->super.super)) {
                ERROR("couldn't create opengl adapter for video effect.");
                return 0;
            }

            adapter->plug_effect(adapter, effect, 1);
            demo->kgo_effect_root = &adapter->super.super;
            self->has_adapter_p = 1;

            if (!effect->new (effect)) {
                ERROR("couldn't create video effect.");
                return 0;
            }
        } else if (frame_type == dict->get_atom(dict, OPENGL_FRAME_TYPE_NAME)) {
            effect_t *effect = demo->kgo_effect_root;

            if (!effect->new (effect)) {
                ERROR("couldn't create video effect.");
                return 0;
            }
        } else {
            ERROR("video effect has invalid frame type: %s",
                   atom_get_cstring_value(frame_type));
            error_p = 1;
        }

        /* for now, no geometry info */
        {
            demo->kgo_effect_root->set_frame_signature(demo->kgo_effect_root,
                                                       NULL);
        }
    }

    return !error_p;
}

kgo_sdlgl_driver_t *kgo_sdlgl_driver_instantiate(kgo_sdlgl_driver_t *x)
{
    kgo_sdlgl_driver_t *sdlgl_driver = kgo_sdlgl_driver_instantiate_super(x);

    sdlgl_driver->super.new = kgo_sdlgl_driver_new;
    sdlgl_driver->super.destroy = kgo_sdlgl_driver_destroy;
    sdlgl_driver->super.has_fd = kgo_sdlgl_driver_has_fd;
    sdlgl_driver->super.get_fd = kgo_sdlgl_driver_get_fd;
    sdlgl_driver->super.get_event_listener =
        kgo_sdlgl_driver_get_event_listener;
    sdlgl_driver->super.update_frame = kgo_sdlgl_driver_update;
    sdlgl_driver->super.get_frame_type = kgo_sdlgl_driver_get_frame_type;
    sdlgl_driver->super.configure_demo = kgo_sdlgl_driver_configure_demo;
    sdlgl_driver->super.allocate_frame = kgo_sdlgl_driver_allocate_frame;

#ifdef LINUX
    sdlgl_driver->super.get_display = kgo_sdlgl_get_display;
    sdlgl_driver->super.get_window = kgo_sdlgl_get_window;
#endif

    return sdlgl_driver;
}

void kgo_sdlgl_driver_initialize() __attribute__((constructor));

void kgo_sdlgl_driver_initialize()
{
    kgo_sdlgl_driver_t *windowed_driver =
        kgo_sdlgl_driver_instantiate_toplevel(NULL);
    kgo_sdlgl_driver_t *fullscreen_driver =
        kgo_sdlgl_driver_instantiate_toplevel(NULL);

    windowed_driver->windowed_p = 1;

    put_kgo_driver("sdlgl-fullscreen", &fullscreen_driver->super);
    put_kgo_driver("sdlgl", &windowed_driver->super);
}
