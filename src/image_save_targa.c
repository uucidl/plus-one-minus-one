/* a10 133
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/image_save_targa.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 133 */




#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_1_1_IMAGE_SAVE_TARGA);

#include "image_save_targa.h"
#include <libc/stdint.h>
#include <libc/stdlib.h>

static inline
void write_le8(int8_t* destination, int8_t i)
{
    destination[0] = i;
}

static inline
void write_le16(int8_t* destination, int16_t i)
{
    #if BYTE_ORDER == LITTLE_ENDIAN
    destination[0] = (i & 0x00ff);
    destination[1] = (i & 0xff00) >> 8;
    #elif BYTE_ORDER == BIG_ENDIAN
    destination[0] = (i & 0xff00) >> 8;
    destination[1] = (i & 0x00ff);
    #else
    #warning "weird byte order."
    #endif
}

static inline
void write_le32(int8_t* destination, int32_t i)
{
    #if BYTE_ORDER == LITTLE_ENDIAN
    write_le16(destination + 0, i & 0xffff);
    write_le16(destination + 2, (i & 0xffff0000) >> 16);
    #elif BYTE_ORDER == BIG_ENDIAN
    write_le16(destination + 0, (i & 0xffff0000) >> 16);
    write_le16(destination + 2, i & 0xffff);
    #else
    #warning "weird byte order."
    #endif
}

void image_save_targa(image_t* image, FILE* fd)
{
    int8_t header[18];

    // ID length
    write_le8(header + 0, 0); // no image id

    // Colormap type
    write_le8(header + 1, 0); // no colormap

    // Image type
    write_le8(header + 2, 2); // uncompressed, True color

    // Colormap specification
    {
	const int n = 5;
	int j;
	for(j=0; j<n; j++)
	    write_le8(header + 3 + j, 0);
    }

    // Image specification
    // x-origin
    write_le16(header + 8,  0);
    // y-origin
    write_le16(header + 10, 0);
    // width
    write_le16(header + 12, image->width);
    // height
    write_le16(header + 14, image->height);
    // pixel depth
    write_le8(header + 16, 24); // rgb
    // descriptor (starts at bottom left, no alpha)
    write_le8(header + 17, 0);

    if(fwrite(header, 18, 1, fd) != 1) {
	ERROR1("couldn't write header.");
	return;
    }

    // now write the lines
    if(image->width > 0) {
	int8_t* line = malloc(image->width * 3);
	int j;
	int error_p = 0;
	for(j=0; j<image->height; j++) {
	    uint32_t* source = image->pixels + j * image->pitch;
	    int i;
	    for(i=0; i<image->width; i++) {
		uint32_t pixel = source[i];
		int8_t* destination = line + i*3;
		write_le8(destination + 0, (pixel & 0xff0000) >> 16);
		write_le8(destination + 1, (pixel & 0x00ff00) >> 8);
		write_le8(destination + 2, (pixel & 0x0000ff));
	    }
	    if(fwrite(line, 3*image->width, 1, fd) != 1) {
		ERROR2("couldn't write line '%d'.", j);
		error_p = 1;
		break;
	    }
	}
	free(line);
	if(error_p)
	    return;
    }
    {
      int8_t footer[26] = "TRUEVISION-XFILE";
      if(fwrite(footer, 26, 1, fd) != 1) {
	ERROR1("couldn't write footer.");
      }
    }
}
