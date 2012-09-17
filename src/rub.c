/* a10 98
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/rub.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 98 */




#include "rub.h"

#include <libc/stdlib.h>
#include <libc/string.h>

static
void rub_computes(effect_t* self, void* content, double ms)
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
	    
	    {
		int r = 0;
		int g = 0;
		int b = 0;
		if( (scanline[x] & 0x00ff0000)<<p > rsum )
		    r = 0x00ff0000;

		if( (scanline[x] & 0x0000ff00)<<p > gsum )
		    g = 0x0000ff00;
		
		if( (scanline[x] & 0x000000ff)<<p > bsum )
		    b = 0x000000ff;
		
		scanline[x] = r | g | b;
	    }
	}
	rsum = gsum = bsum = 0;
	scanline += blur->super.width;
    } }

    scanline = pixels+(blur->super.height-1)*blur->super.width;
    { int y; for(y=blur->super.height; y>0; y--) {
	int x; 
	memset(blur->buffer, 0, sizeof(int32_t)*n);
	for(x=blur->super.width; x>0; x--) {
	    int ex = x-1;
	    if(ex <= blur->super.width-n) {
		rsum -= (blur->buffer[ex & (n-1)] & 0x00ff0000);
		gsum -= (blur->buffer[ex & (n-1)] & 0x0000ff00);
		bsum -= (blur->buffer[ex & (n-1)] & 0x000000ff);
	    }
	    blur->buffer[ex & (n-1)] = scanline[ex];
	    rsum += (scanline[ex] & 0x00ff0000);
	    gsum += (scanline[ex] & 0x0000ff00);
	    bsum += (scanline[ex] & 0x000000ff);

	    {
		int r = 0;
		int g = 0;
		int b = 0;
		if( (scanline[ex] & 0x00ff0000)<<p > rsum )
		    r = 0x00ff0000;

		if( (scanline[ex] & 0x0000ff00)<<p > gsum )
		    g = 0x0000ff00;
		
		if( (scanline[ex] & 0x000000ff)<<p > bsum )
		    b = 0x000000ff;
		
		scanline[ex] = r | g | b;
	    }
	}
	rsum = gsum = bsum = 0;
	scanline -= blur->super.width;
    } }
}

static
void rub_masked_computes(effect_t* self, void* content, double ms)
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
	    if( ((scanline[x] & mask)<<p) > sum) 
		scanline[x] = (0xffffffff & mask) | (scanline[x] & ~mask);
	    else
		scanline[x] = scanline[x] & ~mask;
	}
	sum = 0;
	scanline += blur->super.width;
    } }

    scanline = pixels+(blur->super.height-1)*blur->super.width;
    { int y; for(y=blur->super.height; y>0; y--) {
	int x; 
	memset(blur->buffer, 0, sizeof(int32_t)*n);
	for(x=blur->super.width; x>0; x--) {
	    int ex = x-1;
	    if(ex <= blur->super.width-n) {
	      sum -= (blur->buffer[ex & (n-1)] & mask);
	    }
	    blur->buffer[ex & (n-1)] = scanline[ex];
	    sum += (scanline[ex] & mask);
	    if( ((scanline[ex] & mask)<<p) > sum) 
		scanline[ex] = (0xffffffff & mask) | (scanline[ex] & ~mask);
	    else
		scanline[ex] = scanline[ex] & ~mask;
	}
	sum = 0;
	scanline -= blur->super.width;
    } }
}

static
void rub_set_mode(blur_t* self, enum blur_mode_t mode)
{
  if(mode == MASKED)
      self->super.super.computes = rub_masked_computes;
  else /* mode == NORMAL */
      self->super.super.computes = rub_computes;
}

rub_t* rub_instantiate(rub_t* x)
{
    rub_t* r = rub_instantiate_super (x);

    effect_register_instance("rub", &r->super.super.super);

    r->super.super.super.computes = rub_computes;
    r->super.set_mode = rub_set_mode;

    return r;
}
    
