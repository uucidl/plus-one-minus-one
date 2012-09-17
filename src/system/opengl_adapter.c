/* a10 915
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/opengl_adapter.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 915 */




#include "opengl_adapter.h"
#include <system/video_effect.h>
#include <system/video_to_opengl_frame_converter.h>
#include <system/demo.h>
#include <libc/string.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_SYSTEM_OPENGL_ADAPTER);

static
int opengl_adapter_destroy(effect_t* zelf)
{
    opengl_adapter_t* self = (opengl_adapter_t*) zelf;
    if(self->fc) {
	self->fc->destroy(self->fc);
	object_retire(frame_converter_to_object(self->fc));
	self->fc = NULL;
    }
    
    if(self->frame) {
	a_free(self->frame);
	self->frame = NULL;
    }

    if(self->own_p && self->effect) {
	self->effect->destroy(self->effect);
	object_retire(effect_to_object(self->effect));
    }

    return 1;
}

static
int opengl_adapter_plug_effect(opengl_adapter_t* self, effect_t* e, int own_p)
{
    int status;
    atom_t video_frame_atom = dictionary_get_instance()->get_atom(dictionary_get_instance(), VIDEO_EFFECT_FRAME_TYPE_NAME);
    atom_t frame_type = e->get_frame_type(e);

    if(video_frame_atom == frame_type) {
	INFO1("all is fine and dandy.");
	self->fc = &video_to_opengl_frame_converter_instantiate_toplevel(NULL)->super;
	self->fc->new(self->fc);

	/*
	  we are doing it like this. yes. taking the width and height from demo_get_instance.
	*/
	{
	    frame_converter_frame_signature_t signature;
	    argb32_video_frame_signature_t video_signature;
	    
	    frame_converter_frame_signature_instantiate_toplevel (&signature);
	    argb32_video_frame_signature_instantiate_toplevel (&video_signature);

	    video_signature.width  = demo_get_instance()->video_width;
	    video_signature.height = demo_get_instance()->video_height;
	    video_signature.pitch  = video_signature.width;

	    /*
	      allocate a frame
	    */
	    self->nbytes = sizeof(uint32_t) * video_signature.pitch * video_signature.height;
	    self->frame = a_malloc(self->nbytes);
				   
	    signature.input_signature  = &video_signature.super;
	    signature.output_signature = 0;
	    self->fc->set_frame_signature (self->fc, &signature);

	    e->set_frame_signature (e, &video_signature.super);

	    argb32_video_frame_signature_retire (&video_signature);
	    frame_converter_frame_signature_retire (&signature);
	}
	self->effect = e;
	self->own_p  = !!own_p;
	
	status = 1;
    } else {
	ERROR2("Unknown frame type: '%s'\n", atom_get_cstring_value(frame_type));
	status = 0;
    }

    return status;
}

static
void opengl_adapter_computes(effect_t* zelf, void* content, double ms)
{
    opengl_adapter_t* self = (opengl_adapter_t*) zelf;
    frame_converter_frame_t converter_frame;

    memset(self->frame, 0x0, self->nbytes);
    self->effect->computes(self->effect, self->frame, ms);
    
    converter_frame.input_frame  = self->frame;
    converter_frame.output_frame = content;

    self->fc->computes(self->fc, &converter_frame, ms);
}

opengl_adapter_t* opengl_adapter_instantiate(opengl_adapter_t* x)
{
    opengl_adapter_t* oa = opengl_adapter_instantiate_super (x);

    oa->plug_effect          = opengl_adapter_plug_effect;
    oa->super.super.computes = opengl_adapter_computes;
    oa->super.super.destroy  = opengl_adapter_destroy;

    return oa;
}
