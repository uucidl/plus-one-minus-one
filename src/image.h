/* a10 856
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/image.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 856 */



#ifndef KNOS_DEMOS_1_1_IMAGE_H_
#define KNOS_DEMOS_1_1_IMAGE_H_

#include <libc/stdint.h>
#include <libc/stdio.h>
#include <library/memory.h>

typedef struct image_t
{
    object_t super;
    uint32_t* pixels;
    int width;
    int height;
    int pitch;
} image_t;

CLASS_INHERIT (image, object);

int image_new(image_t* self, unsigned int width,
	      unsigned int height,
	      unsigned int pitch);
int image_destroy(image_t* self);

/*
   image blitting functions: clipping done hence x and y can be
   negative, positive, outside or inside the screen.
*/
void image_blit(image_t* self, int x, int y, uint32_t* pixels,
		int width, int height, int pitch);

/* linear interpolation using the alpha values */
void image_blit_blend(image_t* self, int x, int y, uint32_t* pixels,
		       int width,  int height,
		       int pitch);

void image_blit_scale(image_t* self, int x, int y, float zu, float zv,
		      uint32_t* pixels,  int width,
		       int height,  int pitch);

void image_blit_blend_scale(image_t* self, int x, int y, float zu, float zv,
			    uint32_t* pixels,  int width,
			     int height,  int pitch);

/* u,v are the scanning vectors.
 *
 * normal image rendering for example is obtained with
 * u = (1, 0) and v = (0, 1)
 *
 * rotation is obtained with something like:
 * u = (cos(t), sin(t)) and v = (sin(t), cos(t))
 *
 * zooming is obtained by linear multiplication
 * of u and v by the scaling factor.

 NOT IMPLEMENTED

void image_render_u_v(image_t* self, uint32_t* pixels,
		      int width, int height, int pitch,
		      double u_x, double u_y,
		      double v_x, double v_y);
 */

/* render the quad
   (x1, y1) . (x4, y4) .
   (x2, y2) . (x3, y3) .
   from image to a quad of width 'qwidth' and height 'qheight' on 'pixels'
*/

void image_render_quad(image_t* self,
		       float x1, float y1,
		       float x2, float y2,
		       float x3, float y3,
		       float x4, float y4,
		       uint32_t* pixels, int width, int height, int pitch,
		       int qwidth,  int qheight);

#endif
