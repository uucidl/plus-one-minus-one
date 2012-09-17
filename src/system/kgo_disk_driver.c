/* a10 425
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/kgo_disk_driver.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 425 */




#include "kgo_driver.h"

#include <libc/string.h>
#include <image_save_png.h>
#include <image_save_targa.h>
#include <libc/math.h>
#include <libc/limits.h>
#include <scripting/dictionary.h>
#include <system/video_effect.h>

#include <scripting/dictionary.h>
#include <system/opengl_effect.h>
#include <system/video_adapter.h>

#include "kgo.h"

typedef struct kgo_disk_driver_t
{
    kgo_driver_t super;
    image_t      image;
    unsigned int frame;
    char*  title;

    unsigned int filename_n;
    unsigned int number_n;
    char* filename;
    char* format;
    int started_p;

    int    (*parent_configure_demo)(struct kgo_driver_t* self, demo_t* demo);

    kgo_driver_t* subdriver;
	void* subdriver_frame;
} kgo_disk_driver_t;

CLASS_INHERIT(kgo_disk_driver, kgo_driver);

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_SYSTEM_KGO_DISK_DRIVER);

static
int kgo_disk_driver_new(kgo_driver_t* zelf,
			char* title, unsigned int width, unsigned int height)
{
    kgo_disk_driver_t* self = (kgo_disk_driver_t*) zelf;

    self->image.width  = width;
    self->image.pitch  = width;
    self->image.height = height;
    self->image.pixels = malloc(width * height * sizeof(int32_t));
    if(title == NULL)
	self->title = strdup("");
    else
	self->title = strdup(title);

    // sufficiently large since index is an unsigned int
    self->number_n   = (1 + (int)log10(UINT_MAX));

    {
      const unsigned int n = strlen("%s-%0") + (1 + (int) log10(self->number_n)) + strlen("d.tga") + 1;
      self->format = calloc(n, 1);
      snprintf(self->format, n, "%%s-%%0%dd.tga", self->number_n);

      self->filename_n = strlen(self->title) + 1 + n + self->number_n + 1;
      self->filename   = calloc(self->filename_n, 1);
    }

    self->started_p = 0;

    return !!self->image.pixels;
}

static
int kgo_disk_driver_destroy(struct kgo_driver_t* zelf)
{
    kgo_disk_driver_t* self = (kgo_disk_driver_t*) zelf;

    if (self->subdriver) {
	    self->subdriver->destroy (self->subdriver);
	    kgo_driver_retire (self->subdriver);
	    self->subdriver = NULL;
    }

    if(self->image.pixels) {
	free(self->image.pixels);
	self->image.pixels = NULL;
	self->image.width = 0;
	self->image.height = 0;
    }

    free(self->title);

    if(self->filename) {
	free(self->filename);
	self->filename = NULL;
    }

    if(self->format) {
	free(self->format);
	self->format = NULL;
    }

    return 1;
}

static
int kgo_disk_driver_configure_demo(kgo_driver_t* zelf, demo_t* demo) {
	kgo_disk_driver_t* self = (kgo_disk_driver_t*) zelf;

	int error_p = 0;

	error_p = !demo->kgo_effect_root;

	if (!error_p) {
		atom_t frame_type = demo->kgo_effect_root->get_frame_type (demo->kgo_effect_root);
		dictionary_t* dict = dictionary_get_instance();

		atom_t opengl_frame_type = dict->get_atom (dict, OPENGL_FRAME_TYPE_NAME);

		if (frame_type == opengl_frame_type) {
			/* adapt an opengl effect to the driver */
			self->subdriver = kgo_get_driver (opengl_frame_type);
			if (!self->subdriver) {
				ERROR1("couldn't find an opengl driver to use");
				return 0;
			}

			self->subdriver->new (self->subdriver, self->title, self->image.width, self->image.height);
			self->subdriver->configure_demo (self->subdriver, demo);
			self->subdriver_frame = self->subdriver->allocate_frame (self->subdriver);

			video_adapter_t* adapter = video_adapter_instantiate_toplevel (NULL);
			effect_t* effect = demo->kgo_effect_root;

			if (!adapter->super.super.new (&adapter->super.super)) {
				ERROR1("couldn't create video adapter for opengl effect.");
				return 0;
			}

			if (!effect->new (effect)) {
				ERROR1("couldn't create opengl effect.");
				return 0;
			}

			adapter->plug_effect (adapter, effect, 1);
			demo->kgo_effect_root = &adapter->super.super;
		}
	}

	return self->parent_configure_demo (zelf, demo);
}

static
void kgo_disk_driver_start(struct kgo_driver_t* zelf)
{
    kgo_disk_driver_t* self = (kgo_disk_driver_t*) zelf;

    if (self->subdriver && self->subdriver->start) {
	    self->subdriver->start (self->subdriver);
    }

    self->started_p = 1;
}

static
void kgo_disk_driver_stop(struct kgo_driver_t* zelf)
{
    kgo_disk_driver_t* self = (kgo_disk_driver_t*) zelf;

    if (self->subdriver && self->subdriver->stop) {
	    self->subdriver->stop (self->subdriver);
    }

    self->started_p = 0;
}

static
int kgo_disk_driver_has_fd(struct kgo_driver_t* self)
{
    return 0;
}

static
int kgo_disk_driver_get_fd(struct kgo_driver_t* self)
{
  return -1;
}

static
event_listener_t* kgo_disk_driver_get_event_listener(struct kgo_driver_t* self)
{
    return event_listener_instantiate_toplevel(NULL);
}

void kgo_disk_driver_update_frame(struct kgo_driver_t* zelf, void* frame, int event_pending)
{
    kgo_disk_driver_t* self = (kgo_disk_driver_t*) zelf;
    int32_t* source = (int32_t*) frame;

    if(self->started_p) {
	    if (self->subdriver) {
		self->subdriver->update_frame (self->subdriver, self->subdriver_frame, event_pending);
	    }

	int j;
	for(j=0; j<self->image.height; j++)
	    memcpy(self->image.pixels + j*self->image.pitch, source + j*self->image.width, self->image.width * sizeof(int32_t));
	{
	    FILE* fd;
	    snprintf(self->filename, self->filename_n, self->format, self->title, self->frame);
	    fd = fopen(self->filename, "wb");
	    image_save_targa(&self->image, fd);
	    fclose(fd);

	    self->frame++;
	}
    }
}

static
atom_t kgo_disk_driver_get_frame_type(struct kgo_driver_t* zelf)
{
    return dictionary_get_instance()->new_atom(dictionary_get_instance(), VIDEO_EFFECT_FRAME_TYPE_NAME);
}

static
void* kgo_disk_driver_allocate_frame(struct kgo_driver_t* zelf)
{
    kgo_disk_driver_t* self = (kgo_disk_driver_t*) zelf;
    return a_malloc(sizeof(int32_t) * self->image.width * self->image.height);
}

kgo_disk_driver_t* kgo_disk_driver_instantiate(kgo_disk_driver_t* x)
{
    kgo_disk_driver_t* disk_driver = kgo_disk_driver_instantiate_super (x);

    disk_driver->super.new		  = kgo_disk_driver_new;
    disk_driver->super.destroy		  = kgo_disk_driver_destroy;
    disk_driver->super.start		  = kgo_disk_driver_start;
    disk_driver->super.stop		  = kgo_disk_driver_stop;
    disk_driver->super.has_fd		  = kgo_disk_driver_has_fd;
    disk_driver->super.get_fd		  = kgo_disk_driver_get_fd;
    disk_driver->super.get_event_listener = kgo_disk_driver_get_event_listener;
    disk_driver->super.update_frame	  = kgo_disk_driver_update_frame;
    disk_driver->super.get_frame_type	  = kgo_disk_driver_get_frame_type;
    disk_driver->super.allocate_frame     = kgo_disk_driver_allocate_frame;

    disk_driver->parent_configure_demo = disk_driver->super.configure_demo;
    disk_driver->super.configure_demo = kgo_disk_driver_configure_demo;

    return disk_driver;
}

static
void kgo_disk_driver_initialize() __attribute__((constructor));

static
void kgo_disk_driver_initialize()
{
    kgo_driver_t* driver = &kgo_disk_driver_instantiate_toplevel(NULL)->super;
    put_kgo_driver("disk", driver);
}
