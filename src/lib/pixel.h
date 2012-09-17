/* a10 882
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/pixel.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 882 */



/*
  
some pixel routines

*/

#include <libc/stdint.h>

/*
  linear interpolation between two pixels. 
  result is returned.
  alpha is 0..256
*/
static inline
uint32_t pixel_lerp(uint32_t alpha, const uint32_t d, const uint32_t s)
{
    uint32_t dstrb = d      & 0xFF00FF;
    uint32_t dstag = d >> 8 & 0xFF00FF;
    
    uint32_t srcrb = s      & 0xFF00FF;
    uint32_t srcag = s >> 8 & 0xFF00FF;
    
    uint32_t drb = srcrb - dstrb;
    uint32_t dag = srcag - dstag;
    
    drb *= alpha;  
    dag *= alpha;  
    drb >>= 8;
    dag >>= 8;
    
    {
	const uint32_t rb  = (drb + dstrb)      & 0x00FF00FF;
	const uint32_t ag  = (dag + dstag) << 8 & 0xFF00FF00;
    
	return rb | ag;
    }
}

/*
  add one pixel to another, wraparound.
*/

static inline
uint32_t pixel_add(const uint32_t a, const uint32_t b)
{
    uint32_t a_rb = a      & 0xff00ff;
    uint32_t a_ag = a >> 8 & 0xff00ff;

    uint32_t b_rb = b      & 0xff00ff;
    uint32_t b_ag = b >> 8 & 0xff00ff;

    {
	const uint32_t rb = (a_rb + b_rb)      & 0x00ff00ff;
	const uint32_t ag = (a_ag + b_ag) << 8 & 0xff00ff00;
	
	return rb | ag;
    }
}

/*
  add one pixel to another, saturated.
  destroys alpha.
*/

static inline
uint32_t pixel_add_saturate(const uint32_t a, const uint32_t b)
{
    uint32_t a_rb = a & 0xff00ff;
    uint32_t a_g  = a & 0x00ff00;

    uint32_t b_rb = b & 0xff00ff;
    uint32_t b_g  = b & 0x00ff00;

    {
	uint32_t rb = (a_rb + b_rb);
	uint32_t g  = (a_g + b_g);

	if(rb & 0xff00ff00) { /* needs to saturate */
	    if(rb & 0xff000000)
		rb = 0x00ff0000 | (rb & 0xffff);
	    if(rb & 0xff00)
		rb = 0xff | (rb & 0xff0000);
	}
	if(g & 0x00ff0000) { /* needs to saturate g */
	    g = 0x00ff00;
	}
	
	return rb | g;
    }
}

static
void dim2(int32_t* pixels, unsigned int n) __attribute__((unused)); 

static
void dim2(int32_t* pixels, unsigned int n)
{
    int32_t *pixel = pixels;
    while(n--) {
	int g = *pixel & 0x00fe00;
        *pixel &= 0xfe00fe;
	*pixel >>= 1;
	*pixel |= (g>>1);
	pixel++;
    }
}

static
void add2(int32_t* pixels, unsigned int n) __attribute__((unused)); 

static
void add2(int32_t* pixels, unsigned int n)
{
    int32_t *pixel = pixels;

    while(n--) {
	int g = *pixel & 0x00ff00;
	g <<= 1;
	if(g & 0x00ff0000)
	    g = 0x00ff00;
        *pixel &= 0xff00ff;
	*pixel <<= 1;
	if(*pixel & 0xff000000) { // saturate r
	    *pixel = 0x00ff0000 | (*pixel & 0xffff);
	}
	if(*pixel & 0x00ff00) { // saturate b
	    *pixel = 0xff | (*pixel & 0xff0000);
	}
	*pixel |=  g;
	pixel++;
    }
}

#include <libc/math.h>

static inline
uint32_t grey(unsigned char c)
{
    double r = pow((c/255.0), 0.45) * 1.104 * 255.0;
    double g = pow((c/255.0), 0.45) * 1.074 * 255.0;
    double b = pow((c/255.0), 0.45) * 1.000 * 255.0;
    r = r > 255.0 ? 255.0 : r;
    g = g > 255.0 ? 255.0 : g;
    b = b > 255.0 ? 255.0 : b;

    return 
	(((int) r << 16) & 0xff0000) |
	(((int) g <<  8) & 0x00ff00) |
	((int) b & 0x0000ff);
}

static inline 
uint32_t argb_to_ahsv(const uint32_t argb) 
{ 
    uint8_t max, mid, min;
    unsigned int r = (argb >> 16) & 0xff;
    unsigned int g = (argb >> 8)  & 0xff;
    unsigned int b =  argb        & 0xff;
    uint32_t pixel = argb & 0xff000000;

    // split colors
    if (r >= g) { 
	if (r >= b) { 
	    if (g >= b) { 
		/* r > g > b */
		max = r;
		mid = g;
		min = b;
		r   = 0;
	    } else { 
		/* r > b > g */
		max = r;
		mid = b;
		min = g;
		r   = 5;
	    }
	} else { 
	    /* b > r > g */
	    max = b;
	    mid = r;
	    min = g;
	    r   = 4;
	}
    } else { 
	if (g >= b) { 
	    if (r>=b) { 
		/* g > r > b */
		max = g;
		mid = r;
		min = b;
		r   = 1;
	    } else { 
		/* g > b > r */
		max = g;
		mid = b;
		min = r;
		r   = 2;
	    }
	} else { 
	    /* b > g > r */
	    max = b;
	    mid = g;
	    min = r;
	    r   = 3;
	}
    }
    
    /* sets v */
    pixel |= max;
    
    // colors are split into max,mid,min and base h.
    if (min != max) {
	/* sets s */
	pixel |= (255 - 256 * min / max) << 8;
	
	mid = max - (max * (max - mid)) / (max - min);
	min = 256 * mid / max;  // 0..256
	if (r & 1)
	    min = 256 - min;

	/* sets h */
	pixel |= ((256*r + min) / 6) << 16;
    }
    
    return pixel;
}

static inline
uint32_t ahsv_to_argb(uint32_t ahsv) 
{
    uint32_t pixel = ahsv & 0xff000000;
    unsigned int max, mid, min, h;
    const unsigned int s = (ahsv >> 8) & 0xff;
    unsigned int r, g, b;

    h  = (ahsv >> 16) & 0xff;
    h *= 6;

    max = ahsv & 0xff;

    min = max * (256 - s) / 256;
    mid = max * ((h & 0xff) + 1) / 256;
    if (h & 0x100) 
	mid = max - mid;
    mid = max - (max - mid) * (s + 1) / 256;
    h /= 256;
    
    switch (h) 	{
    case 0:
	r=max; g=mid; b=min; 
	break;
    case 1: 
	r=mid; g=max; b=min; 
	break;
    case 2: 
	r=min; g=max; b=mid; 
	break;
    case 3: r=min; g=mid; b=max; 
	break;
    case 4: 
	r=mid; g=min; b=max; 
	break;
    case 5: 
	r=max; g=min; b=mid; 
	break;
    default:
	// impossible
	r=0; g=0; b=0;
	break;
    }

    pixel |= (r << 16);
    pixel |= (g << 8);
    pixel |= b;
    
    return pixel;
}
