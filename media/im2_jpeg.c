/* uav/groundcontrol/im2_jpeg.c
 * (C) Jeffrey Bridge 2005 - 2009
 *
 * simplified version for use with ELEC301 project
 *
 * the main interface is image_load_jpeg().
 * it is similar to pgmReadFile(), but the args are not identical.
 */

#include <setjmp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <jpeglib.h>

#define IMG_ERR_CORRUPT -3

/* In order to be able to jump back out to our routine during JPEG decoding,
 * we add a setjmp/longjmp buffer to make a custom JPEG error manager struct.
 */
typedef struct {
	struct jpeg_error_mgr pub;
	jmp_buf jmpbuf;
} myJpegErrMgr;

static
void jpegerrmgr_exit(j_common_ptr cinfo)
{
	myJpegErrMgr *mgr = (myJpegErrMgr*)cinfo->err;
	/* don't output the error message, just backtrack out */
	//cinfo->err->output_message(cinfo);
	longjmp(mgr->jmpbuf, 23);
}

#ifdef WIN32
/* The default libjpeg error output on windoze is to pop up a dialog box.
 * We would prefer just a printf.
 */
static
void jpegerrmgr_output(j_common_ptr cinfo)
{
	char buf[JMSG_LENGTH_MAX];
	cinfo->err->format_message(cinfo, buf);
	fprintf(stderr, "%s\n", buf);
}
#endif

void *image_load_jpeg(char *filename,
		      const int require_w, const int require_h,
		      int *width, int *height,
		      int *error)
{
	void *pixels = NULL;
	FILE *infile = NULL;

	/* set up libjpeg structures */
	struct jpeg_decompress_struct cinfo;
	myJpegErrMgr err;

	cinfo.err = jpeg_std_error(&err.pub);
	cinfo.err->error_exit = &jpegerrmgr_exit;
#ifdef WIN32
	cinfo.err->output_message = &jpegerrmgr_output;
#endif
	jpeg_create_decompress(&cinfo);

	/* deep error handling */
	if (setjmp(err.jmpbuf)) {
		if (pixels != NULL) {
			free(pixels);
			pixels = NULL;
		}
		if (error != NULL)
			*error = IMG_ERR_CORRUPT;
		goto bail;
	}

	/* optimize reading based on stream type */
	if ((infile = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		goto bail;
	}
	jpeg_stdio_src(&cinfo, infile);

	/* read basic file info */
	jpeg_read_header(&cinfo, TRUE);
	if (((require_w != 0) && (cinfo.image_width != require_w)) ||
	    ((require_h != 0) && (cinfo.image_height != require_h))) {
		fprintf(stderr, "err: user requested a specific width or height and the file doesn't match\n");
		goto bail;
	}

	if (width != NULL)
		*width = cinfo.image_width;
	if (height != NULL)
		*height = cinfo.image_height;

	/* pick a depth */
	cinfo.out_color_space = JCS_GRAYSCALE;
	cinfo.output_components = 1;

	/* allocate a buffer for the image */
	pixels = malloc(cinfo.image_width * cinfo.image_height * cinfo.output_components);
	if (pixels == NULL)
		goto bail;
	int stride = cinfo.image_width * cinfo.output_components;

	/* decompress the file */
	jpeg_start_decompress(&cinfo);
	uint8_t *rowptr = pixels;
	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, &rowptr, 1);
		rowptr += stride;
	}
	jpeg_finish_decompress(&cinfo);

 bail:
	if (infile != NULL) {
		fclose(infile);
		infile = NULL;
	}
	/* now, clean up */
	jpeg_destroy_decompress(&cinfo);

	return pixels;
}
