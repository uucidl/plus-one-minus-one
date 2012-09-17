/* a10 157
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/freetype/outline.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 157 */




#include "outline.h"

#include "ft_renderer.h"

#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/math.h>

static
int outline_new(outline_t* self)
{
  FT_Outline_New(ft_renderer_get_library(), 0, 0, &self->ftoutline);

  self->ftoutline.flags |= ft_outline_high_precision;

  return 1;
}

static
int outline_destroy(outline_t* self)
{
  FT_Outline_Done(ft_renderer_get_library(), &self->ftoutline);

  return 1;
}

static
void outline_add_point(outline_t* self)
{
  self->ftoutline.n_points++;
  self->ftoutline.points = realloc(self->ftoutline.points, self->ftoutline.n_points*sizeof(FT_Vector));
  self->ftoutline.tags = realloc(self->ftoutline.tags, self->ftoutline.n_points*sizeof(char));
}

static
void outline_add_contour(outline_t* self)
{
  self->ftoutline.n_contours++;
  self->ftoutline.contours = realloc(self->ftoutline.contours, self->ftoutline.n_contours*sizeof(short));
}

static
outline_t* outline_copy(outline_t* self, outline_t* src)
{
    while(self->ftoutline.n_points < src->ftoutline.n_points) {
	outline_add_point(self);
    }
    
    if(src->ftoutline.n_points) {
	memcpy(self->ftoutline.points, src->ftoutline.points, 
	       src->ftoutline.n_points*sizeof(FT_Vector));
	memcpy(self->ftoutline.tags, src->ftoutline.tags,
	       src->ftoutline.n_points*sizeof(char));
    }
    
    while(self->ftoutline.n_contours < src->ftoutline.n_contours) {
	outline_add_contour(self);
    }
    
    if(src->ftoutline.n_points) {
	memcpy(self->ftoutline.contours, src->ftoutline.contours,
	       src->ftoutline.n_contours*sizeof(short));
    }
    
    return self;
}

static inline
FT_Pos float2Pos(float f)
{
    FT_Pos pos = f*64.f;
    return pos;
}

static
outline_t* outline_move_to(outline_t* self, vector2d_t* to)
{
  /* creates a new contour, a new point */
  outline_add_point(self);
  outline_add_contour(self);

  self->ftoutline.points[self->ftoutline.n_points-1].x = float2Pos(to->x);
  self->ftoutline.points[self->ftoutline.n_points-1].y = float2Pos(to->y);
  self->ftoutline.tags[self->ftoutline.n_points-1] = 1;

  return self;
}

static
outline_t* outline_line_to(outline_t* self, vector2d_t* to)
{
  /* adds a new point */
  outline_add_point(self);
  
  self->ftoutline.points[self->ftoutline.n_points-1].x = float2Pos(to->x);
  self->ftoutline.points[self->ftoutline.n_points-1].y = float2Pos(to->y);
  self->ftoutline.tags[self->ftoutline.n_points-1] = 1;

  /* initialize end point of contour to this point
     -- it needs to be the last */
  self->ftoutline.contours[self->ftoutline.n_contours-1] = self->ftoutline.n_points-1; 
  
  return self;
}

static
outline_t* outline_conic_to(outline_t* self, vector2d_t* control, vector2d_t* to)
{
  outline_add_point(self);
  self->ftoutline.points[self->ftoutline.n_points-1].x = float2Pos(control->x);
  self->ftoutline.points[self->ftoutline.n_points-1].y = float2Pos(control->y);
  self->ftoutline.tags[self->ftoutline.n_points-1] = 0;

  outline_add_point(self);
  
  self->ftoutline.points[self->ftoutline.n_points-1].x = float2Pos(to->x);
  self->ftoutline.points[self->ftoutline.n_points-1].y = float2Pos(to->y);
  self->ftoutline.tags[self->ftoutline.n_points-1] = 1;

  /* initialize end point of contour to this point
     -- it needs to be the last */
  self->ftoutline.contours[self->ftoutline.n_contours-1] = self->ftoutline.n_points-1; 

  return self;
}

static
outline_t* outline_cubic_to(outline_t* self, vector2d_t* control1, vector2d_t* control2, vector2d_t* to)
{
  outline_add_point(self);
  self->ftoutline.points[self->ftoutline.n_points-1].x = float2Pos(control1->x);
  self->ftoutline.points[self->ftoutline.n_points-1].y = float2Pos(control1->y);
  self->ftoutline.tags[self->ftoutline.n_points-1] = 2;

  outline_add_point(self);
  self->ftoutline.points[self->ftoutline.n_points-1].x = float2Pos(control2->x);
  self->ftoutline.points[self->ftoutline.n_points-1].y = float2Pos(control2->y);
  self->ftoutline.tags[self->ftoutline.n_points-1] = 2;

  outline_add_point(self);
  
  self->ftoutline.points[self->ftoutline.n_points-1].x = float2Pos(to->x);
  self->ftoutline.points[self->ftoutline.n_points-1].y = float2Pos(to->y);
  self->ftoutline.tags[self->ftoutline.n_points-1] = 1;

  /* initialize end point of contour to this point
     -- it needs to be the last */
  self->ftoutline.contours[self->ftoutline.n_contours-1] = self->ftoutline.n_points-1; 

  return self;
}

static
outline_t* outline_scale(outline_t* self, vector2d_t* scaling)
{
  self->transform.xx = scaling->x * 65536;
  self->transform.xy = 0;
  self->transform.yx = 0;
  self->transform.yy = scaling->y * 65536;

  FT_Outline_Transform(&self->ftoutline, &self->transform);

  return self;
}

static
outline_t* outline_rotate(outline_t* self, double angle)
{
    self->transform.xy = (-sin(angle)) * 65536;
    self->transform.xx = (cos(angle))  * 65536;
    self->transform.yy = (cos(angle))  * 65536;
    self->transform.yx = (sin(angle))  * 65536;

  FT_Outline_Transform(&self->ftoutline, &self->transform);

  return self;
}

static
outline_t* outline_translate(outline_t* self, vector2d_t* dis)
{
    FT_Outline_Translate(&self->ftoutline, float2Pos(dis->x), float2Pos(dis->y));
    
    return self;
}

outline_t* outline_instantiate(outline_t* x)
{
  outline_t* o = outline_instantiate_super (x);
 
  o->new = outline_new;
  o->destroy = outline_destroy;
  o->copy = outline_copy;

  o->move_to   = outline_move_to;
  o->line_to   = outline_line_to;
  o->conic_to  = outline_conic_to;
  o->cubic_to  = outline_cubic_to;
  o->scale     = outline_scale;
  o->rotate    = outline_rotate;
  o->translate = outline_translate;

  return o;
}
