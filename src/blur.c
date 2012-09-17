/* a10 147
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/blur.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 147 */




#include "blur.h"

#include <libc/math.h>
#include <libc/stdlib.h>
#include <libc/stdint.h>
#include <libc/string.h>

#include <lib/pixel.h>

/* this is the chunk at which we allocate the self->buffer at */
#define CHUNK 256 

static
int blur_new(effect_t* self)
{
    blur_t* blur = (blur_t*) self;
    blur->n	       = 0;
    blur->buffer       = a_malloc(CHUNK * sizeof(int32_t));
    blur->buffer_chunk = 1;
    blur->mask	       = 0x0;	/* 'do nothing' mask */
   
    return 1;
}

static
int blur_destroy(effect_t* self)
{
    blur_t* blur = (blur_t*) self;
    if(blur->buffer)
	a_free(blur->buffer);
    
    return 1;
}

static 
int blur_set_length(blur_t* self, int length)
{ 
    int n = length>0 ? length:0;
    int p = 0;

    if(n > 0) {
      p = log(n) / log(2);
      n = pow(2, p);

      if(n != self->n) {
	  int new_buffer_chunk = self->buffer_chunk;
	  while(n > new_buffer_chunk * CHUNK) {
	      new_buffer_chunk++;
	  }
	  
	  if(new_buffer_chunk != self->buffer_chunk) {
	    a_free(self->buffer);
	    self->buffer = a_malloc
	      (new_buffer_chunk * CHUNK * sizeof(int32_t));
	    self->buffer_chunk = new_buffer_chunk;
	  }
      }
    }

    self->p = p;
    self->n = n;
    
    return n;
}

/* default routine */
static
void blur_computes(effect_t* self, void* content, double ms)
{
    blur_t* blur = (blur_t*) self;
    int32_t* __restrict__ pixels = content;
    int32_t* __restrict__ scanline;
    int rsum = 0;
    int gsum = 0;
    int bsum = 0;
    const int n = blur->n;
    const int p = blur->p;

    if(n <= 0)
	return;

    scanline = pixels;
    { int y; for(y=0; y<blur->super.height; y++) {
	int x;
	memset(blur->buffer, 0, sizeof(int32_t)*n);
	for(x=0; x<blur->super.width; x++) {
	    if(x >= n) {
		rsum -= (blur->buffer[x & (n-1)] & 0x00ff0000);
		gsum -= (blur->buffer[x & (n-1)] & 0x0000ff00);
		bsum -= (blur->buffer[x & (n-1)] & 0x000000ff);
	    }
	    blur->buffer[x & (n-1)] = scanline[x];
	    rsum += (scanline[x] & 0x00ff0000);
	    gsum += (scanline[x] & 0x0000ff00);
	    bsum += (scanline[x] & 0x000000ff);
	    scanline[x] = 
		((rsum>>p) & 0xff0000) | 
		((gsum>>p) & 0x00ff00) | 
		((bsum>>p) & 0x0000ff);
	}
	rsum = gsum = bsum = 0;
	scanline += blur->super.pitch;
    } }

    scanline = pixels+(blur->super.height-1)*blur->super.pitch;
    { int y; for(y=blur->super.height; y>0; y--) {
	int x; 
	memset(blur->buffer, 0, sizeof(int32_t)*n);
	for(x=blur->super.width; x>0; x--) {
	    int ex = x-1;
	    if(ex <= blur->super.width-n-1) {
		rsum -= (blur->buffer[ex & (n-1)] & 0x00ff0000);
		gsum -= (blur->buffer[ex & (n-1)] & 0x0000ff00);
		bsum -= (blur->buffer[ex & (n-1)] & 0x000000ff);
	    }
	    blur->buffer[ex & (n-1)] = scanline[ex];
	    rsum += (scanline[ex] & 0x00ff0000);
	    gsum += (scanline[ex] & 0x0000ff00);
	    bsum += (scanline[ex] & 0x000000ff);
	    scanline[ex] = 
		((rsum>>p) & 0xff0000) | 
		((gsum>>p) & 0x00ff00) | 
		((bsum>>p) & 0x0000ff);
	}
	rsum = gsum = bsum = 0;
	scanline -= blur->super.pitch;
    } }
}

/* this function works only if length < 256 */

static
void blur_le256_computes(effect_t* self, void* content, double ms)
{
    blur_t* blur = (blur_t*) self;
    int32_t* __restrict__ pixels = content;
    int32_t* __restrict__ scanline;
    int rbsum = 0;
    int agsum = 0;
    const int n = blur->n;
    const int p = blur->p;

    if(n <= 0)
	return;
    else if(n >= 256) {
	blur_computes(self, content, ms);
	return;
    }

    scanline = pixels;
    { int y; for(y=0; y<blur->super.height; y++) {
	int x;
	memset(blur->buffer, 0, sizeof(int32_t)*n);
	for(x=0; x<blur->super.width; x++) {
	    if(x >= n) {
		rbsum -= (blur->buffer[x & (n-1)] & 0x00ff00ff);
		agsum -= (blur->buffer[x & (n-1)] & 0xff00ff00);
	    }
	    blur->buffer[x & (n-1)] = scanline[x];
	    rbsum += (scanline[x] & 0x00ff00ff);
	    agsum += (scanline[x] & 0xff00ff00);
	    scanline[x] = 
	      ((rbsum>>p) & 0xff00ff) | ((agsum>>p) & 0xff00ff00);
	}
	rbsum = agsum = 0;
	scanline += blur->super.pitch;
    } }

    scanline = pixels+(blur->super.height-1)*blur->super.pitch;
    { int y; for(y=blur->super.height; y>0; y--) {
	int x; 
	memset(blur->buffer, 0, sizeof(int32_t)*n);
	for(x=blur->super.width; x>0; x--) {
	    int ex = x-1;
	    if(ex <= blur->super.width-n-1) {
		rbsum -= (blur->buffer[ex & (n-1)] & 0x00ff00ff);
		agsum -= (blur->buffer[ex & (n-1)] & 0xff00ff00);
	    }
	    blur->buffer[ex & (n-1)] = scanline[ex];
	    rbsum += (scanline[ex] & 0x00ff00ff);
	    agsum += (scanline[ex] & 0xff00ff00);
	    scanline[ex] = ((rbsum>>p) & 0xff00ff) | ((agsum>>p) & 0xff00ff00);
	}
	rbsum = agsum = 0;
	scanline -= blur->super.pitch;
    } }
}

static
void blur_masked_computes(effect_t* self, void* content, double ms)
{
    blur_t* blur = (blur_t*) self;
    int32_t* __restrict__ pixels = content;
    int32_t* __restrict__ scanline;
    int sum = 0;
    const int n = blur->n;
    const int p = blur->p;
    const int mask = blur->mask;

    if(n <= 0)
	return;

    scanline = pixels;
    { int y; for(y=0; y<blur->super.height; y++) {
	int x;
	memset(blur->buffer, 0, sizeof(int32_t)*n);
	for(x=0; x<blur->super.width; x++) {
	    if(x >= n) {
	      sum -= (blur->buffer[x & (n-1)] & mask);
	    }
	    blur->buffer[x & (n-1)] = scanline[x];
	    sum += (scanline[x] & mask);
	    scanline[x] = ((sum>>p) & mask) | (scanline[x] & ~mask);
	}
	sum = 0;
	scanline += blur->super.pitch;
    } }

    scanline = pixels+(blur->super.height-1)*blur->super.pitch;
    { int y; for(y=blur->super.height; y>0; y--) {
	int x; 
	memset(blur->buffer, 0, sizeof(int32_t)*n);
	for(x=blur->super.width; x>0; x--) {
	    int ex = x-1;
	    if(ex <= blur->super.width-n-1) {
	      sum -= (blur->buffer[ex & (n-1)] & mask);
	    }
	    blur->buffer[ex & (n-1)] = scanline[ex];
	    sum += (scanline[ex] & mask);
	    scanline[ex] = ((sum>>p) & mask) | (scanline[ex] & ~mask);
	}
	sum = 0;
	scanline -= blur->super.pitch;
    } }
}

static
void blur_set_mode(blur_t* self, enum blur_mode_t mode)
{
  if(mode == MASKED)
    self->super.super.computes = blur_masked_computes;
  else /* mode == NORMAL */
    self->super.super.computes = blur_le256_computes;
}

static
void blur_set_mask(blur_t* self, int mask)
{
  self->mask = mask;
}

blur_t* blur_instantiate(blur_t* x)
{
    blur_t* b = blur_instantiate_super (x);

    effect_register_instance ("blur", &b->super.super);

    b->super.super.new	    = blur_new;
    b->super.super.destroy  = blur_destroy;
    b->super.super.computes = blur_le256_computes;
    b->set_length	    = blur_set_length;
    b->set_mode		    = blur_set_mode;
    b->set_mask		    = blur_set_mask;

    return b;
}

#undef CHUNK
