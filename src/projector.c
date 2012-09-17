/* a10 148
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/projector.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 148 */




#include <system/effects.h>
#include <libc/stdlib.h>
#include <libc/stdio.h>
#include <libc/string.h>

#include "image.h"
#include "image_load_jpeg.h"
#include "image_load_png.h"
#include "projector.h"

#include <lib/url_open.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_PROJECTOR);
 
static
int projector_new(effect_t* self)
{
    projector_t* proj = (projector_t*) self;
    proj->sources     = malloc (sizeof(source_t) * 40);
    proj->next_source = 0;
    proj->sources_n   = 40;

    return 1;
}

static
int projector_destroy(effect_t* zelf)
{
    projector_t* self = (projector_t*) zelf;
    int i;

    for(i=0; i<self->next_source; i++) {
	image_t* im = &self->sources[i].source;
	image_destroy(im);
    }
    if(self->sources_n) {
	free(self->sources);
	self->sources = NULL;
    }

    return 1;
}

static
source_t* add_source(projector_t* self)
{
    if(self->next_source == self->sources_n) {
	int old_n = self->sources_n;
	self->sources_n += 1;
	self->sources = realloc(self->sources, sizeof(source_t)*self->sources_n);
	memset(self->sources + old_n, 0, sizeof(source_t)*(self->sources_n - old_n));
    }
    
    self->next_source += 1;

    return &self->sources[self->next_source-1];
}

static
int projector_add_image(projector_t* self, stream_t* fd)
{
    image_t* x;
    source_t* source = NULL;
    int64_t pos;
    int index = 0;

    if(!fd)
	return 0;
    
    pos = stream_get_callbacks(fd)->tell(fd);

    source = add_source(self);
    x = image_load_png(&source->source, fd);
    if(x != NULL) {
	index = self->next_source;
	self->set_current(self, index);
    } else {
	stream_get_callbacks(fd)->seek(fd, pos, SEEK_SET);
	
	x = image_load_jpg(&source->source, fd);
	if(x != NULL) {
	    index = self->next_source;
	    self->set_current(self, index);
	} else {
	    /* failure to load an image! */
	    self->next_source--;
	    index = 0;
	}
    }

    self->set_normalized_magnification(self, index, 1.0f);
    source->zoom_p = 0;
    
    return index;
}

static
void projector_set_current(projector_t* self, int i)
{
    if(i > self->next_source) {
	i = self->next_source;
    } else if(i <= 0) {
	i = 1;
    }
    
    self->current = i-1;
}

static
void projector_computes(effect_t* self, void* content, double ms)
{
    projector_t* proj = (projector_t*) self;
    uint32_t* __restrict__ pixels = content;
    image_t* image;
    source_t* source;
    
    if(proj->sources) {
	source = &proj->sources[proj->current];
	image = &source->source;
    } else
	return;

    if(image->pixels && image->width && image->height) {
	/* center image: */
	int x = proj->super.width - image->width;
	int y = proj->super.height - image->height;
	int do_not_scale_p = !source->zoom_p;

	/* test if aspect ratio is roughly similar to output
	if(!do_not_scale_p) {
	    float aspekt = 1.0f*image->width / image->height;
	    float raspekt = 1.0f*proj->super.width / proj->super.height;
	    if(aspekt*raspekt >= 2.0f || aspekt*raspekt <= 0.5f)
		do_not_scale_p = 1;
	    else
		do_not_scale_p = 0;
	}
	*/

	if(do_not_scale_p) {
	    image_blit_blend(image, x/2, y/2, 
			     pixels, 
			     proj->super.width, proj->super.height, proj->super.pitch);
	} else {
	    image_blit_blend_scale(image, source->x/2, source->y/2, 
				   source->zoom, source->zoom,
				   pixels, 
				   proj->super.width, proj->super.height, proj->super.pitch);
	}
    }
}

static
void projector_set_normalized_magnification(projector_t* self, int i, float z)
{
    source_t* source = NULL;
    image_t*  image = NULL;
    float     ratio = 1.0f; /* normalized ratio */

    if(i > 0 && i <= self->next_source) {
	source = &self->sources[i-1];
    } else 
	return;

    image = &source->source;

    DEBUG5("image: %dx%d, effect: %dx%d\n", image->width, image->height, self->super.width, self->super.height);

    if(image->width > image->height) {
	ratio = 1.0f*self->super.width / image->width;
	source->x = -self->super.width * (z - 1.0f);
	source->y = self->super.height - image->height * ratio * z;
	DEBUG3("top-right offset: %dx%d\n", source->x, source->y);
    } else {
	ratio = 1.0f*self->super.height / image->height;
	source->x = self->super.width - image->width * ratio * z;
	source->y = -self->super.height * (z - 1.0f);
    }

    source->zoom_p = 1;
    source->zoom = ratio * z;
}

projector_t* projector_instantiate(projector_t* x)
{
    projector_t* proj = projector_instantiate_super (x);
    
    proj->super.super.new = projector_new;
    proj->super.super.destroy = projector_destroy;
    proj->super.super.computes = projector_computes;
    proj->add_image = projector_add_image;
    proj->set_current = projector_set_current;
    proj->set_normalized_magnification = projector_set_normalized_magnification;
    
    return proj;
}
