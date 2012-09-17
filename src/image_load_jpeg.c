/* a10 743
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/image_load_jpeg.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 743 */




#include "image_load_jpeg.h"

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_1_1_IMAGE_LOAD_JPEG);

#include <libc/stdlib.h>
#include <libc/endian.h>
#include <libc/setjmp.h>

#include <jpeglib.h>

/* image loader for libjpeg */

/*
  we must first define a data source manager for libjpeg, using our stream_t
*/
#include <jerror.h>

typedef struct {
    struct jpeg_source_mgr pub;

    stream_t* stream;
    JOCTET* buffer;
    char start_of_file_p;
} stream_source_mgr;

#define INPUT_BUF_SIZE  4096	/* choose an efficiently fread'able size */

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

METHODDEF(void)
init_source (j_decompress_ptr cinfo)
{
    stream_source_mgr* src = (stream_source_mgr*) cinfo->src;

    /* We reset the empty-input-file flag for each image,
     * but we don't clear the input buffer.
     * This is correct behavior for reading a series of images from one source.
     */
    src->start_of_file_p = 1;
}

/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
    stream_source_mgr* src = (stream_source_mgr*) cinfo->src;
    size_t nbytes;

    nbytes = stream_get_callbacks(src->stream)
      ->read(src->buffer, 1, INPUT_BUF_SIZE, src->stream);

    if (nbytes <= 0) {
	/* original: Treat empty input file as fatal error
	if (src->start_of_file_p)
	    ERREXIT(cinfo, JERR_INPUT_EMPTY);
	*/
	ERREXIT(cinfo, JERR_INPUT_EOF);
	WARNMS(cinfo, JWRN_JPEG_EOF);
	/* Insert a fake EOI marker */
	src->buffer[0] = (JOCTET) 0xFF;
	src->buffer[1] = (JOCTET) JPEG_EOI;
	nbytes = 2;
    }

    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = nbytes;
    src->start_of_file_p     = FALSE;

    return TRUE;
}

/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
    stream_source_mgr* src = (stream_source_mgr*) cinfo->src;

    /* Just a dumb implementation for now.  Could use fseek() except
     * it doesn't work on pipes.  Not clear that being smart is worth
     * any trouble anyway --- large skips are infrequent.
     */
    if (num_bytes > 0) {
	while (num_bytes > (long) src->pub.bytes_in_buffer) {
	    num_bytes -= (long) src->pub.bytes_in_buffer;
	    (void) fill_input_buffer(cinfo);
	    /* note we assume that fill_input_buffer will never return FALSE,
	     * so suspension need not be handled.
	     */
	}
	src->pub.next_input_byte += (size_t) num_bytes;
	src->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
}

/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */
METHODDEF(void)
term_source (j_decompress_ptr cinfo) {
    /* no work necessary here */
}

/*
 * Prepare for input from a stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */

GLOBAL(void)
jpeg_stream_src (j_decompress_ptr cinfo, stream_t* infile)
{
  stream_source_mgr* src;

  /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if (cinfo->src == NULL) {	/* first time for this JPEG object? */
      cinfo->src = (struct jpeg_source_mgr *)
	  (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				      sizeof(stream_source_mgr));
      src = (stream_source_mgr*) cinfo->src;
      src->buffer = (JOCTET *)
	  (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				      INPUT_BUF_SIZE * sizeof(JOCTET));
  }

  src = (stream_source_mgr*) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->stream = infile;
  src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = NULL; /* until buffer loaded */
}

typedef struct {
    struct jpeg_error_mgr super;

    jmp_buf env;
    volatile image_t* im; // eventually, image to cleanup
    volatile int image_is_owned_p;
} myerror_mgr;

static
void image_load_jpeg_error_exit(j_common_ptr cinfo)
{
    struct jpeg_decompress_struct* c = (struct jpeg_decompress_struct*) cinfo ;
    myerror_mgr* mgr = (myerror_mgr*) c->err;

    char buffer [JMSG_LENGTH_MAX];
    (*cinfo->err->format_message) (cinfo, buffer);

    // prints the error message.
    printf ("error message: %s\n", buffer);

    if(mgr->im) {
	image_destroy((image_t*) mgr->im);
	if(mgr->image_is_owned_p)
	    free((void*) mgr->im);
    }

    ERROR1("jpeg load error.");

    longjmp(mgr->env, 1);
}

image_t* image_load_jpg(image_t* x, stream_t* stream)
{
  image_t* im;

  TRACE1 ("loading jpeg file");

  if (!stream) {
    im = NULL;
  } else {
    int load;
    im = image_instantiate_toplevel (x);

    struct jpeg_decompress_struct cinfo;
    myerror_mgr errmgr;

    errmgr.im = im;
    errmgr.image_is_owned_p = (im != x);

    if (setjmp (errmgr.env)) {
	    jpeg_destroy_decompress(&cinfo);

	// coming back from an error
	return NULL;
    }

    /* insert here useless error test */
    /* jpeg object initialisation */
    jpeg_create_decompress (&cinfo);

    /* specify data source */
    jpeg_stream_src (&cinfo, stream);

    /* standard error */
    cinfo.err               = jpeg_std_error (&errmgr.super);
    errmgr.super.error_exit = image_load_jpeg_error_exit;

    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
	    return NULL;
    }

    if (jpeg_start_decompress(&cinfo) < 0) {
	    return NULL;
    }

    if (!cinfo.output_width || !cinfo.output_height) {
	    WARNING1("null size image.");
	    return NULL;
    }

    image_new(im,
	      cinfo.output_width,
	      cinfo.output_height,
	      cinfo.output_width);

    switch (cinfo.output_components) {
    case 1: /* GRAY */
	load = 1;
	break;
    case 3: // RGB
	load = 1;
	break;
    default:
	/* don't know how to load that */
	image_destroy (im);
	if(im != x) {
		image_retire (im);
	}
	im = NULL;

	load = 0;
    }

    if (load) {
	JSAMPARRAY scanline_buf;

	scanline_buf = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE,
		 cinfo.output_width *
		 cinfo.output_components,
		 cinfo.rec_outbuf_height);

	while (cinfo.output_scanline < cinfo.output_height) {
	    int n;

	    uint32_t* dest = im->pixels + im->pitch * cinfo.output_scanline;

	    n = jpeg_read_scanlines(&cinfo,
				    scanline_buf,
				    cinfo.rec_outbuf_height);

	    int s;
	    for (s = 0; s < n; s++) {
		    unsigned char* scanline = scanline_buf [s];

		    /* copy to image */
		    int i;
		    for (i = 0; i < im->width; i++) {
			    if(cinfo.output_components == 1) {
				    dest[i] = 0xff000000 |
					    (scanline[i] << 16) |
					    (scanline[i] << 8) |
					    (scanline[i]);
			    } else if(cinfo.output_components == 3) {
#if (BYTE_ORDER == LITTLE_ENDIAN && defined (PIXEL_RGBA8888)) ||	\
	(BYTE_ORDER == BIG_ENDIAN && defined (PIXEL_BGRA8888))
				    dest[i] =
					    0xff000000 |
					    (scanline [3*i + 0] << 16) |
					    (scanline [3*i + 1] << 8) |
					    (scanline [3*i + 2] << 0);
#else
				    /*dest[i] = 0xff000000 | //alpha
					    (scanline [3*i + 2] << 0) |
					    (scanline [3*i + 1] << 8) |
					    (scanline [3*i + 2] << 16);
				    */
				    dest[i] = 0xff000000 | // alpha
					    (scanline [3 * i + 0] << 0) | // r
					    (scanline [3 * i + 1] << 8) | // g
					    (scanline [3 * i + 2] << 16); // b

#endif
			    }
		    }
		    dest += im->pitch;
	    }
	}
    }

    jpeg_finish_decompress(&cinfo);

  }

    return im;
}
