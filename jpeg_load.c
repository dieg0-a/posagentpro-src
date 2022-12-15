#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>

extern void initialize(int width, int height, int components);
extern void write_scanline(unsigned char *buffer);


/******************** JPEG DECOMPRESSION SAMPLE INTERFACE *******************/

/* This half of the example shows how to read data from the JPEG decompressor.
 * It's a bit more refined than the above, in that we show:
 *   (a) how to modify the JPEG library's standard error-reporting behavior;
 *   (b) how to allocate workspace using the library's memory manager.
 *
 * Just to make this example a little different from the first one, we'll
 * assume that we do not intend to put the whole image into an in-memory
 * buffer, but to send it line-by-line someplace else.  We need a one-
 * scanline-high JSAMPLE array as a work buffer, and we will let the JPEG
 * memory manager allocate it for us.  This approach is actually quite useful
 * because we don't need to remember to deallocate the buffer separately: it
 * will go away automatically when the JPEG object is cleaned up.
 */


/*
 * ERROR HANDLING:
 *
 * The JPEG library's standard error handler (jerror.c) is divided into
 * several "methods" which you can override individually.  This lets you
 * adjust the behavior without duplicating a lot of code, which you might
 * have to update with each future release.
 *
 * Our example here shows how to override the "error_exit" method so that
 * control is returned to the library's caller when a fatal error occurs,
 * rather than calling exit() as the standard error_exit method does.
 *
 * We use C's setjmp/longjmp facility to return control.  This means that the
 * routine which calls the JPEG library must first execute a setjmp() call to
 * establish the return point.  We want the replacement error_exit to do a
 * longjmp().  But we need to make the setjmp buffer accessible to the
 * error_exit routine.  To do this, we make a private extension of the
 * standard JPEG error handler object.  (If we were using C++, we'd say we
 * were making a subclass of the regular error handler.)
 *
 * Here's the extended error handler struct:
 */


struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

struct my_error_mgr;
typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}



/*
 * Sample routine for JPEG decompression.  We assume that the source file name
 * is passed in.  We want to return 1 on success, 0 on error.
 */

GLOBAL(int)
jpeg_initialize (struct jpeg_decompress_struct *cinfo)
{
    jpeg_create_decompress(cinfo);
    return 1;
}


GLOBAL(int)
read_JPEG_file (const char *data, size_t length,struct jpeg_decompress_struct *cinfo, JSAMPARRAY *output, struct my_error_mgr *jerr)
{
  cinfo->err = jpeg_std_error(&jerr->pub);
  jerr->pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr->setjmp_buffer)) {
  /* If we get here, the JPEG code has signaled an error.
  * We need to clean up the JPEG object, close the input file, and return.
  */
  jpeg_destroy_decompress(cinfo);
  return 0;
  }

//  jpeg_create_decompress(cinfo);
  jpeg_mem_src(cinfo, (const unsigned char *)data, length);

  (void) jpeg_read_header(cinfo, TRUE);
//  cinfo->scale_denom = 2;
//  cinfo->scale_num = 1;

  (void) jpeg_start_decompress(cinfo);

  int row_stride = cinfo->output_width * cinfo->output_components;

  if (output != NULL) *output = (*cinfo->mem->alloc_sarray)
        ((j_common_ptr) cinfo, JPOOL_IMAGE, row_stride, cinfo->output_height);
  else return 0;

  while (cinfo->output_scanline < cinfo->output_height) {
    (void) jpeg_read_scanlines(cinfo, (*(output) + cinfo->output_scanline), 1);
  }
  return 1;
}

