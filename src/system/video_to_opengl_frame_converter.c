/* a10 750
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/video_to_opengl_frame_converter.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 750 */




#include "video_to_opengl_frame_converter.h"

#include <system/video_frame.h> // for frame definition
#include <libc/string.h>
#include <third-party/opengl/gl.h>
#include <third-party/opengl/glu.h>

static
int video_to_opengl_frame_converter_destroy(frame_converter_t* zelf)
{
    video_to_opengl_frame_converter_t* self = (video_to_opengl_frame_converter_t*) zelf;

    glDeleteTextures(1, &self->texture_id);

    if(self->framebuffer) {
	a_free(self->framebuffer);
	self->framebuffer = NULL;
    }
    if(self->buffer) {
	a_free(self->buffer);
	self->buffer = NULL;
    }
    
    return 1;
}

static
void video_to_opengl_frame_converter_activate_filter (video_to_opengl_frame_converter_t* self, pixel_filter_type_t filter) {
	switch (filter) {
	case BILINEAR_FILTER:
		self->activate_bilinear_filter_p = 1;
		break;
	case POINT_FILTER:
		self->activate_bilinear_filter_p = 0;
		break;
	}
}

static
void video_to_opengl_frame_converter_set_frame_signature(frame_converter_t* zelf, frame_converter_frame_signature_t* sig)
{
    video_to_opengl_frame_converter_t* self = (video_to_opengl_frame_converter_t*) zelf;
    argb32_video_frame_signature_t* input_sig = (argb32_video_frame_signature_t*) sig->input_signature;

    self->width  = input_sig->width;
    self->height = input_sig->height;
    self->pitch  = input_sig->pitch;

    if(self->width != self->pitch) {
	if (self->buffer) {
	    a_free (self->buffer);
	    self->buffer = NULL;
	}
	/* we'll need a temporary buffer between the frame and the texture */
	self->buffer = a_malloc(sizeof(uint32_t) * self->width * self->height);
    } else {
	if(self->buffer) {
	    a_free(self->buffer);
	}
	self->buffer = NULL;
    }

    {
	int fb_width, fb_height;
	int ideal_width, ideal_height;
	int i;
    
	ideal_width  = self->width;
	ideal_height = self->height;
		
	/* find ideal power of 2 size */
	fb_width = 1;
	while( (fb_width <<= 1) <= ideal_width);
	
	fb_height = 1;
	while( (fb_height <<= 1) <= ideal_height);
	
	if (self->framebuffer) {
		a_free (self->framebuffer);
		self->framebuffer = NULL;
	}

	self->framebuffer = a_malloc(sizeof(int32_t) * fb_width * fb_height);
	for(i=0; i< fb_width*fb_height; i++) {
	    self->framebuffer[i] = 0;
	}

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &self->texture_id);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, self->texture_id);

	glTexImage2D(GL_TEXTURE_2D, 0, 
		     GL_RGBA, 
		     fb_width, fb_height, 
		     0, 
		     GL_RGBA, 
		     GL_UNSIGNED_BYTE, self->framebuffer);
	
	GLint filtering;
	if (self->activate_bilinear_filter_p) {
		filtering = GL_LINEAR;
	} else {
		filtering = GL_NEAREST;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glDisable(GL_TEXTURE_2D);

	self->tex_width  = fb_width;
	self->tex_height = fb_height;
	self->tex_w = 1.f * ideal_width / self->tex_width;
	self->tex_h = 1.f * ideal_height / self->tex_height;
    }

}

void video_to_opengl_frame_converter_computes(frame_converter_t* zelf, void* content, double ms)
{
    video_to_opengl_frame_converter_t* self = (video_to_opengl_frame_converter_t*) zelf;
    frame_converter_frame_t* frame = (frame_converter_frame_t*) content;

    uint32_t* pixels = (uint32_t*) frame->input_frame;
    uint32_t* source;

    /* expand into buffer if needed */
    if(self->width == self->pitch) {
	source = pixels;
    } else {
	/* the annoying way! copy each scanline into a temporary linear buffer */
	int j;
	for(j=0; j<self->height; j++) {
	    memcpy(self->buffer + j*self->width, pixels + j*self->pitch, sizeof(uint32_t)*self->width);
	}
	source = self->buffer;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, self->texture_id);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( -1.0, 1.0, 1., -1., -1., 1.);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
		    self->width, self->height, 
			GL_BGRA,
		//		    GL_BGRA_EXT, /* in fact ARGB,*/ 
		    //		    GL_UNSIGNED_BYTE, 
		    GL_UNSIGNED_INT_8_8_8_8_REV,
		    source);

    glBegin(GL_QUADS);
    glTexCoord2f(0.f, 0.f);
    glVertex2f(-1.f, -1.f);

    glTexCoord2f(0.f, self->tex_h);
    glVertex2f(-1.f, 1.f);
    
    glTexCoord2f(self->tex_w, self->tex_h);
    glVertex2f(1.f, 1.f);
    
    glTexCoord2f(self->tex_w, 0.f);
    glVertex2f(1.f, -1.f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

video_to_opengl_frame_converter_t* video_to_opengl_frame_converter_instantiate
                                      (video_to_opengl_frame_converter_t* x)
{
    video_to_opengl_frame_converter_t* fc = video_to_opengl_frame_converter_instantiate_super (x);

    fc->super.destroy		  = video_to_opengl_frame_converter_destroy;
    fc->super.set_frame_signature = video_to_opengl_frame_converter_set_frame_signature;
    fc->super.computes		  = video_to_opengl_frame_converter_computes;

    fc->activate_filter = video_to_opengl_frame_converter_activate_filter;

    return fc;
}
