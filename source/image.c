
/************************************************************************************
***
***	Copyright 2010-2020 Dell Du(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Sat Jul 31 14:19:59 HKT 2010
***
************************************************************************************/

#include "image.h"

#include <stdlib.h>
#include <errno.h>

#ifdef CONFIG_JPEG
#include <jpeglib.h>
#include <jerror.h>
#endif

#define IMAGE_MAGIC MAKE_FOURCC('I','M','A','G')

#define CLAHE_MAX_ROWS 8
#define CLAHE_MAX_COLS 8

#define IMAGE_MAX_NB_SIZE 25
RGBA_8888 *__image_rgb_nb[IMAGE_MAX_NB_SIZE];

extern int color_rgbcmp(RGBA_8888 * c1, RGBA_8888 * c2);
extern void color_rgbsort(int n, RGBA_8888 * cv[]);
extern int image_memsize(WORD h, WORD w);
extern void image_membind(IMAGE * img, WORD h, WORD w);


#ifdef CONFIG_JPEG
static void __jpeg_errexit(j_common_ptr cinfo)
{
	cinfo->err->output_message(cinfo);
	exit(EXIT_FAILURE);
}
#endif

static int __nb3x3_map(IMAGE * img, int r, int c)
{
	int i, j, k;
	k = 0;
	for (i = -1; i <= 1; i++) {
		for (j = -1; j <= 1; j++) {
			__image_rgb_nb[k] = &(img->base[(r + i) * img->width + c + j]);
			++k;
		}
	}
	color_rgbsort(3 * 3, __image_rgb_nb);
	return RET_OK;
}

static void __draw_hline(IMAGE * img, int *x, int *y, int run_length, int x_advance, int r, int g, int b)
{
	int i;

	if (run_length < 0)
		run_length *= -1;

	for (i = 0; i < run_length; i++) {
		img->ie[*y][*x + (i * x_advance)].r = r;
		img->ie[*y][*x + (i * x_advance)].g = g;
		img->ie[*y][*x + (i * x_advance)].b = b;
	}

	*x += run_length * x_advance;
	*y += 1;
}

static void __draw_vline(IMAGE * img, int *x, int *y, int run_length, int x_advance, int r, int g, int b)
{
	int i;

	if (run_length < 0)
		run_length *= -1;

	for (i = 0; i < run_length; i++) {
		img->ie[*y + i][*x].r = r;
		img->ie[*y + i][*x].g = g;
		img->ie[*y + i][*x].b = b;
	}

	*x += x_advance;
	*y += run_length;
}

static int __color_rgbfind(RGBA_8888 * c, int n, RGBA_8888 * cv[])
{
	int i, k;

	k = color_rgbcmp(c, cv[n / 2]);
	if (k == 0)
		return n / 2;
	if (k < 0) {
		for (i = n / 2 - 1; i >= 0; i--) {
			if (color_rgbcmp(c, cv[i]) == 0)
				return i;
		}
	}
	// k > 0
	for (i = n / 2 + 1; i < n; i++) {
		if (color_rgbcmp(c, cv[i]) == 0)
			return i;
	}
	// not found

	return -1;
}

static void *__image_malloc(WORD h, WORD w)
{
	void *img = (IMAGE *) calloc((size_t) 1, image_memsize(h, w));
	if (!img) {
		syslog_error("Allocate memeory.");
		return NULL;
	}
	return img;
}

extern int text_puts(IMAGE * image, int r, int c, char *text, int color);

int image_memsize(WORD h, WORD w)
{
	int size;

	size = sizeof(IMAGE);
	size += (h * w) * sizeof(RGBA_8888);
	size += h * sizeof(RGBA_8888 *);
	return size;
}

// mem align !
void image_membind(IMAGE * img, WORD h, WORD w)
{
	int i;
	void *base = (void *) img;

	img->magic = IMAGE_MAGIC;
	img->height = h;
	img->width = w;
	img->base = (RGBA_8888 *) (base + sizeof(IMAGE));	// Data
	img->ie = (RGBA_8888 **) (base + sizeof(IMAGE) + (h * w) * sizeof(RGBA_8888));	// Skip head and data
	for (i = 0; i < h; i++)
		img->ie[i] = &(img->base[i * w]);

	// Blank image, for png is more import
	memset(img->base, 255, (h * w) * sizeof(RGBA_8888));
}

IMAGE *image_create(WORD h, WORD w)
{
	void *base = __image_malloc(h, w);
	if (!base) {
		return NULL;
	}
	image_membind((IMAGE *) base, h, w);
	return (IMAGE *) base;
}

int image_clear(IMAGE * img)
{
	check_image(img);
	memset(img->base, 0, img->height * img->width * sizeof(RGBA_8888));
	return RET_OK;
}

BYTE image_getvalue(IMAGE * img, char oargb, int r, int c)
{
	BYTE n;
	switch (oargb) {
	case 'A':
		color_rgb2gray(img->ie[r][c].r, img->ie[r][c].g, img->ie[r][c].b, &n);
		return n;
		break;
	case 'R':
		return img->ie[r][c].r;
		break;
	case 'G':
		return img->ie[r][c].g;
		break;
	case 'B':
		return img->ie[r][c].b;
		break;
	case 'H':					// Hue !!!
		{
			BYTE s, v;
			color_rgb2hsv(img->ie[r][c].r, img->ie[r][c].g, img->ie[r][c].b, &n, &s, &v);
			return n;
		}
		break;
	}
	return 0;
}

void image_setvalue(IMAGE * img, char oargb, int r, int c, BYTE x)
{
	switch (oargb) {
	case 'A':
		img->ie[r][c].r = x;
		img->ie[r][c].g = x;
		img->ie[r][c].b = x;
		break;
	case 'R':
		img->ie[r][c].r = x;
		break;
	case 'G':
		img->ie[r][c].g = x;
		break;
	case 'B':
		img->ie[r][c].b = x;
		break;
	}
}

MATRIX *image_getplane(IMAGE * img, char oargb)
{
	RECT rect;

	image_rect(&rect, img);
	return image_rect_plane(img, oargb, &rect);
}

MATRIX *image_rect_plane(IMAGE * img, char oargb, RECT * rect)
{
	BYTE n;
	int i, j;
	int y2;
	MATRIX *mat;

	CHECK_IMAGE(img);
	CHECK_ARGBH(oargb);

	// BUG: image_rect(rect,img);

	mat = matrix_create(rect->h, rect->w);
	CHECK_MATRIX(mat);

	switch (oargb) {
	case 'Y':
		for (i = 0; i < (rect)->h; i++) {
			if (i + rect->r < 0 || i + rect->r >= img->height)
				continue;
			for (j = 0; j < (rect)->w; j++) {
				if (j + rect->c < 0 || j + rect->c >= img->width)
					continue;
				y2 = (66 * (img->ie[i + rect->r][j + rect->c].r)
					  + 129 * (img->ie[i + rect->r][j + rect->c].g)
					  + 25 * (img->ie[i + rect->r][j + rect->c].b) + 128) / 256 + 16;
				mat->me[i][j] = (double) CLAMP(y2, 0, 255);
			}
		}
		break;
	case 'A':
		for (i = 0; i < (rect)->h; i++) {
			if (i + rect->r < 0 || i + rect->r >= img->height)
				continue;
			for (j = 0; j < (rect)->w; j++) {
				if (j + rect->c < 0 || j + rect->c >= img->width)
					continue;
				color_rgb2gray(img->ie[i + rect->r][j + rect->c].r,
							   img->ie[i + rect->r][j + rect->c].g, img->ie[i + rect->r][j + rect->c].b, &n);
				mat->me[i][j] = (double) n;
			}
		}
		break;
	case 'R':
		for (i = 0; i < (rect)->h; i++) {
			if (i + rect->r < 0 || i + rect->r >= img->height)
				continue;
			for (j = 0; j < (rect)->w; j++) {
				if (j + rect->c < 0 || j + rect->c >= img->width)
					continue;
				mat->me[i][j] = (double) img->ie[i + rect->r][j + rect->c].r;
			}
		}
		break;
	case 'G':
		for (i = 0; i < (rect)->h; i++) {
			if (i + rect->r < 0 || i + rect->r >= img->height)
				continue;
			for (j = 0; j < (rect)->w; j++) {
				if (j + rect->c < 0 || j + rect->c >= img->width)
					continue;
				mat->me[i][j] = (double) img->ie[i + rect->r][j + rect->c].g;
			}
		}
		break;
	case 'B':
		for (i = 0; i < (rect)->h; i++) {
			if (i + rect->r < 0 || i + rect->r >= img->height)
				continue;
			for (j = 0; j < (rect)->w; j++) {
				if (j + rect->c < 0 || j + rect->c >= img->width)
					continue;
				mat->me[i][j] = (double) img->ie[i + rect->r][j + rect->c].b;
			}
		}
		break;
	case 'a':
		for (i = 0; i < (rect)->h; i++) {
			if (i + rect->r < 0 || i + rect->r >= img->height)
				continue;
			for (j = 0; j < (rect)->w; j++) {
				if (j + rect->c < 0 || j + rect->c >= img->width)
					continue;
				mat->me[i][j] = (double) img->ie[i + rect->r][j + rect->c].a;
			}
		}
		break;
	}

	return mat;
}

int image_setplane(IMAGE * img, char oargb, MATRIX * mat)
{
	int i, j;

	check_image(img);
	check_argb(oargb);
	check_matrix(mat);

	switch (oargb) {
	case 'A':
		for (i = 0; i < (int) img->height && i < (int) mat->m; i++) {
			for (j = 0; j < (int) img->width && j < (int) mat->n; j++) {
				img->ie[i][j].r = CLAMP(mat->me[i][j], 0, 255);
				img->ie[i][j].g = CLAMP(mat->me[i][j], 0, 255);
				img->ie[i][j].b = CLAMP(mat->me[i][j], 0, 255);
			}
		}
		break;
	case 'R':
		for (i = 0; i < img->height && i < mat->m; i++) {
			for (j = 0; j < img->width && j < mat->n; j++) {
				img->ie[i][j].r = CLAMP(mat->me[i][j], 0, 255);
			}
		}
		break;
	case 'G':
		for (i = 0; i < img->height && i < mat->m; i++) {
			for (j = 0; j < img->width && j < mat->n; j++) {
				img->ie[i][j].g = CLAMP(mat->me[i][j], 0, 255);
			}
		}
		break;
	case 'B':
		for (i = 0; i < img->height && i < mat->m; i++) {
			for (j = 0; j < img->width && j < mat->n; j++) {
				img->ie[i][j].b = CLAMP(mat->me[i][j], 0, 255);
			}
		}
		break;
	}

	return RET_OK;
}

int image_valid(IMAGE * img)
{
	return (!img || img->height < 1 || img->width < 1 || !img->ie || img->magic != IMAGE_MAGIC) ? 0 : 1;
}

// (i + di, j + dj) will move to outside ?
int image_outdoor(IMAGE * img, int i, int di, int j, int dj)
{
	return (i + di < 0 || i + di >= img->height || j + dj < 0 || j + dj >= img->width);
}

int image_rectclamp(IMAGE * img, RECT * rect)
{
	if (!image_valid(img) || !rect)
		return RET_ERROR;

	rect->r = CLAMP(rect->r, 0, img->height - 1);
	rect->h = CLAMP(rect->h, 0, img->height - rect->r);
	rect->c = CLAMP(rect->c, 0, img->width - 1);
	rect->w = CLAMP(rect->w, 0, img->width - rect->c);

	return RET_OK;
}

void image_destroy(IMAGE * img)
{
	if (!image_valid(img))
		return;
	free(img);
}

#ifdef CONFIG_JPEG
static IMAGE *image_loadjpeg(char *fname)
{
	JSAMPARRAY lineBuf;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr err_mgr;
	int bytes_per_pixel;
	FILE *fp = NULL;
	IMAGE *img = NULL;
	int i, j;

	if ((fp = fopen(fname, "rb")) == NULL) {
		syslog_error("Open file %s.", fname);
		goto read_fail;
	}

	cinfo.err = jpeg_std_error(&err_mgr);
	err_mgr.error_exit = __jpeg_errexit;

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, 1);
	cinfo.do_fancy_upsampling = 0;
	cinfo.do_block_smoothing = 0;
	jpeg_start_decompress(&cinfo);

	bytes_per_pixel = cinfo.output_components;
	lineBuf = cinfo.mem->alloc_sarray((j_common_ptr) & cinfo, JPOOL_IMAGE, (cinfo.output_width * bytes_per_pixel), 1);
	img = image_create(cinfo.output_height, cinfo.output_width);
	if (!img) {
		syslog_error("Create image.");
		goto read_fail;
	}

	if (bytes_per_pixel == 4) {
		img->format = IMAGE_RGBA;
		for (i = 0; i < img->height; ++i) {
			jpeg_read_scanlines(&cinfo, lineBuf, 1);
			for (j = 0; j < img->width; j++) {
				img->ie[i][j].r = lineBuf[0][4 * j];
				img->ie[i][j].g = lineBuf[0][4 * j + 1];
				img->ie[i][j].b = lineBuf[0][4 * j + 2];
				img->ie[i][j].a = lineBuf[0][4 * j + 3];
			}
		}
	} else if (bytes_per_pixel == 3) {
		img->format = IMAGE_RGBA;
		for (i = 0; i < img->height; ++i) {
			jpeg_read_scanlines(&cinfo, lineBuf, 1);
			for (j = 0; j < img->width; j++) {
				img->ie[i][j].r = lineBuf[0][3 * j];
				img->ie[i][j].g = lineBuf[0][3 * j + 1];
				img->ie[i][j].b = lineBuf[0][3 * j + 2];
				// img->ie[i][j].a = 0;
			}
		}
	} else if (bytes_per_pixel == 1) {
		img->format = IMAGE_GRAY;
		for (i = 0; i < img->height; ++i) {
			jpeg_read_scanlines(&cinfo, lineBuf, 1);
			for (j = 0; j < img->width; j++) {
				img->ie[i][j].r = lineBuf[0][j];
				img->ie[i][j].g = img->ie[i][j].r;
				img->ie[i][j].b = img->ie[i][j].r;
				// img->ie[i][j].a = 0;
			}
		}
	} else {
		syslog_error("Color channels is %d (1 or 3).", bytes_per_pixel);
		goto read_fail;
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(fp);

	return img;
  read_fail:
	if (fp)
		fclose(fp);
	if (img)
		image_destroy(img);

	return NULL;
}

// save alpha channel？
static int image_savejpeg(IMAGE * img, const char *filename, int quality)
{
	int row_stride;
	FILE *outfile;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	JSAMPLE *img_buffer;

	if (!image_valid(img)) {
		syslog_error("Bad image.");
		return RET_ERROR;
	}
	if ((outfile = fopen(filename, "wb")) == NULL) {
		syslog_error("Create file (%s).", filename);
		return RET_ERROR;
	}
	img_buffer = (JSAMPLE *) img->base;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = img->width;
	cinfo.image_height = img->height;
	cinfo.input_components = 4;
	cinfo.in_color_space = JCS_EXT_RGBA;	// JCS_RGB; 
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);

	jpeg_start_compress(&cinfo, TRUE);

	cinfo.next_scanline = 0;
	row_stride = img->width * 4;
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &img_buffer[cinfo.next_scanline * row_stride];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	fclose(outfile);

	jpeg_destroy_compress(&cinfo);

	return RET_OK;
}
#endif

#ifdef CONFIG_PNG
#include <png.h>
IMAGE *image_loadpng(char *fname)
{
	FILE *fp;
	IMAGE *image = NULL;
	png_struct *png_ptr = NULL;
	png_info *info_ptr = NULL;
	png_byte buf[8];
	png_byte *png_pixels = NULL;
	png_byte **row_pointers = NULL;
	png_byte *pix_ptr = NULL;
	png_uint_32 row_bytes;

	png_uint_32 width, height;
	int bit_depth, channels, color_type, alpha_present, ret;
	png_uint_32 i, row, col;

	if ((fp = fopen(fname, "rb")) == NULL) {
		syslog_error("Loading PNG file (%s). error no: %d", fname, errno);
		goto read_fail;
	}

	/* read and check signature in PNG file */
	ret = fread(buf, 1, 8, fp);
	if (ret != 8 || !png_check_sig(buf, 8)) {
		syslog_error("Png check sig");
		return NULL;
	}

	/* create png and info structures */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		syslog_error("Png create read struct");
		return NULL;			/* out of memory */
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		syslog_error("Png create info struct");
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return NULL;			/* out of memory */
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		syslog_error("Png jmpbuf");
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return NULL;
	}

	/* set up the input control for C streams */
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);	/* we already read the 8 signature bytes */

	/* read the file information */
	png_read_info(png_ptr, info_ptr);

	/* get size and bit-depth of the PNG-image */
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

	/* 
	 * set-up the transformations
	 */

	/* transform paletted images into full-color rgb */
	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png_ptr);
	/* expand images to bit-depth 8 (only applicable for grayscale images) */
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand(png_ptr);
	/* transform transparency maps into full alpha-channel */
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_expand(png_ptr);

#ifdef NJET
	/* downgrade 16-bit images to 8 bit */
	if (bit_depth == 16)
		png_set_strip_16(png_ptr);
	/* transform grayscale images into full-color */
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);
	/* only if file has a file gamma, we do a correction */
	if (png_get_gAMA(png_ptr, info_ptr, &file_gamma))
		png_set_gamma(png_ptr, (double) 2.2, file_gamma);
#endif

	/* all transformations have been registered; now update info_ptr data,
	 * get rowbytes and channels, and allocate image memory */

	png_read_update_info(png_ptr, info_ptr);

	/* get the new color-type and bit-depth (after expansion/stripping) */
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

	/* calculate new number of channels and store alpha-presence */
	if (color_type == PNG_COLOR_TYPE_GRAY)
		channels = 1;
	else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		channels = 2;
	else if (color_type == PNG_COLOR_TYPE_RGB)
		channels = 3;
	else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		channels = 4;
	else
		channels = 0;			/* should never happen */

	alpha_present = (channels - 1) % 2;

	/* row_bytes is the width x number of channels x (bit-depth / 8) */
	row_bytes = png_get_rowbytes(png_ptr, info_ptr);

	if ((png_pixels = (png_byte *) malloc(row_bytes * height * sizeof(png_byte))) == NULL) {
		syslog_error("Allocate memeory.");
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return NULL;
	}

	if ((row_pointers = (png_byte **) malloc(height * sizeof(png_bytep))) == NULL) {
		syslog_error("Allocate memeory.");
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		free(png_pixels);
		png_pixels = NULL;
		return NULL;
	}

	/* set the individual row_pointers to point at the correct offsets */
	for (i = 0; i < (height); i++)
		row_pointers[i] = png_pixels + i * row_bytes;

	/* now we can go ahead and just read the whole image */
	png_read_image(png_ptr, row_pointers);

	/* read rest of file and get additional chunks in info_ptr - REQUIRED */
	png_read_end(png_ptr, info_ptr);

	/* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);

	/* write data to PNM file */
	pix_ptr = png_pixels;

	image = image_create(height, width);

	for (row = 0; row < height; row++) {
		for (col = 0; col < width; col++) {
			if (bit_depth == 16) {
				image->ie[row][col].r = (((BYTE) * pix_ptr++ << 8) + (BYTE) * pix_ptr++);
				if (channels >= 2)
					image->ie[row][col].g = (((BYTE) * pix_ptr++ << 8) + (BYTE) * pix_ptr++);
				if (channels >= 3)
					image->ie[row][col].b = (((BYTE) * pix_ptr++ << 8) + (BYTE) * pix_ptr++);
				if (channels >= 4) {
					if (alpha_present)
						image->ie[row][col].a = (((BYTE) * pix_ptr++ << 8) + (BYTE) * pix_ptr++);
				}
			} else {
				image->ie[row][col].r = (BYTE) * pix_ptr++;
				if (channels >= 2)
					image->ie[row][col].g = (BYTE) * pix_ptr++;
				if (channels >= 3)
					image->ie[row][col].b = (BYTE) * pix_ptr++;
				if (channels >= 4) {
					if (alpha_present)
						image->ie[row][col].a = (BYTE) * pix_ptr++;
				}
			}
		}
	}

	// Sorted chanels
	if (channels < 2) {
		image_foreach(image, row, col)
			image->ie[row][col].g = image->ie[row][col].r;
		image_foreach(image, row, col)
			image->ie[row][col].b = image->ie[row][col].r;
	}
	if (channels < 3) {
		image_foreach(image, row, col)
			image->ie[row][col].b = image->ie[row][col].r;
	}

	if (row_pointers != (BYTE **) NULL)
		free(row_pointers);
	if (png_pixels != (BYTE *) NULL)
		free(png_pixels);

	fclose(fp);
	return image;

  read_fail:
	if (fp)
		fclose(fp);
	if (image)
		image_destroy(image);

	return NULL;
}

static int image_savepng(IMAGE * img, const char *filename)
{
	int i, j;
	FILE *outfile;
	png_struct *png_ptr = NULL;
	png_info *info_ptr = NULL;

	if (!image_valid(img)) {
		syslog_error("Bad image.");
		return RET_ERROR;
	}
	// Save png alpha channel for display
	if (img->format != IMAGE_MASK) {
		image_foreach(img, i, j)
			img->ie[i][j].a = 255;
	}

	if ((outfile = fopen(filename, "wb")) == NULL) {
		syslog_error("Create file (%s).", filename);
		return RET_ERROR;
	}
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		syslog_error("Png create write struct");
		return RET_ERROR;		/* out of memory */
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		syslog_error("Png create info struct");
		png_destroy_write_struct(&png_ptr, NULL);
		fclose(outfile);
		return RET_ERROR;		/* out of memory */
	}
	if (setjmp(png_jmpbuf(png_ptr))) {
		syslog_error("Png jmpbuf");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(outfile);
		return RET_ERROR;
	}
	png_init_io(png_ptr, outfile);

	png_set_IHDR(png_ptr, info_ptr, img->width, img->height, 8 * sizeof(BYTE) /*bit_depth */ ,
				 PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, (png_bytep *) img->ie);
	png_write_end(png_ptr, NULL);
	fclose(outfile);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	return RET_OK;
}
#endif

IMAGE *image_load(char *fname)
{
	char *extname = strrchr(fname, '.');
	if (extname) {
#ifdef CONFIG_JPEG
		if (strcasecmp(extname, ".jpg") == 0 || strcasecmp(extname, ".jpeg") == 0)
			return image_loadjpeg(fname);
#endif
#ifdef CONFIG_PNG
		if (strcasecmp(extname, ".png") == 0)
			return image_loadpng(fname);
#endif
	}

	syslog_error("Only support jpg/jpeg/png loading.");
	return NULL;
}


int image_save(IMAGE * img, const char *fname)
{
	char *extname = strrchr(fname, '.');
	check_image(img);

	if (extname) {
#ifdef CONFIG_JPEG
		if (strcasecmp(extname, ".jpg") == 0 || strcasecmp(extname, ".jpeg") == 0)
			return image_savejpeg(img, fname, 100);
#endif
#ifdef CONFIG_PNG
		if (strcasecmp(extname, ".png") == 0)
			return image_savepng(img, fname);
#endif
	}

	syslog_error("ONLY Support jpg/jpeg/png image saving.");
	return RET_ERROR;
}

IMAGE *image_copy(IMAGE * img)
{
	IMAGE *copy;

	if (!image_valid(img)) {
		syslog_error("Bad image.");
		return NULL;
	}

	if ((copy = image_create(img->height, img->width)) == NULL) {
		syslog_error("Create image.");
		return NULL;
	}
	memcpy(copy->base, img->base, img->height * img->width * sizeof(RGBA_8888));
	if (img->format == IMAGE_MASK) {
		copy->format = img->format;
		copy->K = img->K;
		memcpy(copy->KColors, img->KColors, ARRAY_SIZE(img->KColors) * sizeof(int));
		copy->KRadius = img->KRadius;
		copy->KInstance = img->KInstance;
		copy->opc = img->opc;
	}
	return copy;
}

IMAGE *image_zoom(IMAGE * img, int nh, int nw, int method)
{
	int i, j, i2, j2;
	double di, dj, d1, d2, d3, d4, u, v, d;
	IMAGE *copy;

	CHECK_IMAGE(img);
	copy = image_create(nh, nw);
	CHECK_IMAGE(copy);

	di = 1.0 * img->height / copy->height;
	dj = 1.0 * img->width / copy->width;
	if (method == ZOOM_METHOD_BLINE) {
		/**********************************************************************
		d1    d2
		    (p)
		d3    d4
		f(i+u,j+v) = (1-u)(1-v)f(i,j) + (1-u)vf(i,j+1) + u(1-v)f(i+1,j) + uvf(i+1,j+1)
		**********************************************************************/
		image_foreach(copy, i, j) {
			i2 = (int) (di * i);
			u = di * i - i2;
			j2 = (int) (dj * j);
			v = dj * j - j2;
			if (i2 == img->height - 1 || j2 == img->width - 1) {
				copy->ie[i][j].r = img->ie[i2][j2].r;
				copy->ie[i][j].g = img->ie[i2][j2].g;
				copy->ie[i][j].b = img->ie[i2][j2].b;
			} else {
				// Red
				d1 = img->ie[i2][j2].r;
				d2 = img->ie[i2][j2 + 1].r;
				d3 = img->ie[i2 + 1][j2].r;
				d4 = img->ie[i2 + 1][j2 + 1].r;
				d = (1.0 - u) * (1.0 - v) * d1 + (1.0 - u) * v * d2 + u * (1.0 - v) * d3 + u * v * d4;
				copy->ie[i][j].r = (int) d;
				// Green
				d1 = img->ie[i2][j2].g;
				d2 = img->ie[i2][j2 + 1].g;
				d3 = img->ie[i2 + 1][j2].g;
				d4 = img->ie[i2 + 1][j2 + 1].g;
				d = (1.0 - u) * (1.0 - v) * d1 + (1.0 - u) * v * d2 + u * (1.0 - v) * d3 + u * v * d4;
				copy->ie[i][j].g = (int) d;
				// Blue
				d1 = img->ie[i2][j2].b;
				d2 = img->ie[i2][j2 + 1].b;
				d3 = img->ie[i2 + 1][j2].b;
				d4 = img->ie[i2 + 1][j2 + 1].b;
				d = (1.0 - u) * (1.0 - v) * d1 + (1.0 - u) * v * d2 + u * (1.0 - v) * d3 + u * v * d4;
				copy->ie[i][j].b = (int) d;
			}
		}
	} else {
		image_foreach(copy, i, j) {
			i2 = (int) (di * i);
			j2 = (int) (dj * j);
			copy->ie[i][j].r = img->ie[i2][j2].r;
			copy->ie[i][j].g = img->ie[i2][j2].g;
			copy->ie[i][j].b = img->ie[i2][j2].b;
		}
	}

	return copy;
}

int image_drawrect(IMAGE * img, RECT * rect, int color, int fill)
{
	int i, j;
	BYTE r, g, b;

	check_image(img);
	image_rectclamp(img, rect);

	r = RGB_R(color);
	g = RGB_G(color);
	b = RGB_B(color);

	if (fill) {
		rect_foreach(rect, i, j) {
			img->ie[rect->r + i][rect->c + j].r = r;
			img->ie[rect->r + i][rect->c + j].g = g;
			img->ie[rect->r + i][rect->c + j].b = b;
		}
	} else {
		// ||
		for (i = rect->r; i < rect->r + rect->h; i++) {
			img->ie[i][rect->c].r = r;
			img->ie[i][rect->c + rect->w - 1].r = r;
			img->ie[i][rect->c].g = g;
			img->ie[i][rect->c + rect->w - 1].g = g;
			img->ie[i][rect->c].b = b;
			img->ie[i][rect->c + rect->w - 1].b = b;
		}

		// ==
		for (j = rect->c; j < rect->c + rect->w; j++) {
			img->ie[rect->r][j].r = r;
			img->ie[rect->r + rect->h - 1][j].r = r;
			img->ie[rect->r][j].g = g;
			img->ie[rect->r + rect->h - 1][j].g = g;
			img->ie[rect->r][j].b = b;
			img->ie[rect->r + rect->h - 1][j].b = b;
		}
	}

	return RET_OK;
}

int image_drawline(IMAGE * img, int r1, int c1, int r2, int c2, int color)
{
	int i, j;
	BYTE r, g, b;
	int temp, adj_up, adj_down, error_term, x_advance, x_delta, y_delta, whole_step,
		initial_pixel_count, final_pixel_count, run_length;

	check_image(img);
	r = RGB_R(color);
	g = RGB_G(color);
	b = RGB_B(color);

	r1 = CLAMP(r1, 0, img->height - 1);
	r2 = CLAMP(r2, 0, img->height - 1);
	c1 = CLAMP(c1, 0, img->width - 1);
	c2 = CLAMP(c2, 0, img->width - 1);

	/* draw top to bottom */
	if (r1 > r2) {
		temp = r1;
		r1 = r2;
		r2 = temp;
		temp = c1;
		c1 = c2;
		c2 = temp;
	}

	/* Figure out whether we're going left or right, and how far we're going horizontally. */
	if ((x_delta = c2 - c1) < 0) {
		x_advance = -1;
		x_delta = -x_delta;
	} else {
		x_advance = 1;
	}

	/* Figure out how far we're going vertically */
	y_delta = r2 - r1;

	/* Special-case horizontal, vertical, and diagonal lines, for speed and to avoid nasty boundary conditions and division by 0. */
	if (x_delta == 0) {			/* Vertical line */
		for (i = 0; i <= y_delta; i++) {
			img->ie[r1 + i][c1].r = r;
			img->ie[r1 + i][c1].g = g;
			img->ie[r1 + i][c1].b = b;
		}
		return RET_OK;
	}

	if (y_delta == 0) {			/* Horizontal line */
		if (x_advance < 0) {
			for (i = x_delta; i >= 0; i += x_advance) {
				img->ie[r1][c1 - i].r = r;
				img->ie[r1][c1 - i].g = g;
				img->ie[r1][c1 - i].b = b;
			}
		} else {
			for (i = 0; i <= x_delta; i += x_advance) {
				img->ie[r1][c1 + i].r = r;
				img->ie[r1][c1 + i].g = g;
				img->ie[r1][c1 + i].b = b;
			}
		}
		return RET_OK;
	}

	if (x_delta == y_delta) {
		/* Diagonal line */
		for (i = 0; i <= x_delta; i++) {
			j = (i * x_advance);
			img->ie[r1 + i][c1 + j].r = r;
			img->ie[r1 + i][c1 + j].g = g;
			img->ie[r1 + i][c1 + j].b = b;
		}
		return RET_OK;
	}

	/* Determine whether the line is X or Y major, and handle accordingly */
	if (x_delta >= y_delta) {
		/* X major line */
		/* Minimum # of pixels in a run in this line */
		whole_step = x_delta / y_delta;

		/* 
		 * Error term adjust each time Y steps by 1; used to tell 
		 * when one extra pixel should be drawn as part of a run, 
		 * to account for fractional steps along the X axis per 
		 * 1-pixel steps along y
		 */
		adj_up = (x_delta % y_delta) * 2;

		/* 
		 * Error term adjust when the error term turns over, used 
		 * to factor out the X step made at that time
		 */
		adj_down = y_delta * 2;

		/* 
		 * Initial error term; reflects an inital step of 0.5 along 
		 * the Y axis
		 */
		error_term = (x_delta % y_delta) - (y_delta * 2);

		/* 
		 * The initial and last runs are partial, because Y advances 
		 * only 0.5 for these runs, rather than 1.  Divide one full 
		 * run, plus the initial pixel, between the initial and last 
		 * runs
		 */
		initial_pixel_count = (whole_step / 2) + 1;
		final_pixel_count = initial_pixel_count;

		/* 
		 * If the basic run length is even and there's no fractional 
		 * advance, we have one pixel that could go to either the 
		 * inital or last partial run, which we'll arbitrarily allocate
		 * to the last run
		 */
		if ((adj_up == 0) && ((whole_step & 0x01) == 0))
			initial_pixel_count--;

		/* 
		 * If there're an odd number of pixels per run, we have 1 pixel
		 * that can't be allocated to either the initial or last 
		 * partial run, so we'll add 0.5 to error term so this pixel 
		 * will be handled by the normal full-run loop
		 */
		if ((whole_step & 0x01) != 0)
			error_term += y_delta;

		/* Draw the first, partial run of pixels */
		__draw_hline(img, &c1, &r1, initial_pixel_count, x_advance, r, g, b);

		/* Draw all full runs */
		for (i = 0; i < (y_delta - 1); i++) {
			/* run is at least this long */
			run_length = whole_step;

			/* 
			 * Advance the error term and add an extra pixel if 
			 * the error term so indicates
			 */
			if ((error_term += adj_up) > 0) {
				run_length++;
				/* reset the error term */
				error_term -= adj_down;
			}

			/* Draw this scan line's run */
			__draw_hline(img, &c1, &r1, run_length, x_advance, r, g, b);
		}

		/* Draw the final run of pixels */
		__draw_hline(img, &c1, &r1, final_pixel_count, x_advance, r, g, b);
		return RET_OK;
	} else {
		/* Y major line */

		/* Minimum # of pixels in a run in this line */
		whole_step = y_delta / x_delta;

		/* 
		 * Error term adjust each time X steps by 1; used to tell when 
		 * 1 extra pixel should be drawn as part of a run, to account 
		 * for fractional steps along the Y axis per 1-pixel steps 
		 * along X
		 */
		adj_up = (y_delta % x_delta) * 2;

		/* 
		 * Error term adjust when the error term turns over, used to 
		 * factor out the Y step made at that time
		 */
		adj_down = x_delta * 2;

		/* Initial error term; reflects initial step of 0.5 along the 
		 * X axis 
		 */
		error_term = (y_delta % x_delta) - (x_delta * 2);

		/* 
		 * The initial and last runs are partial, because X advances 
		 * only 0.5 for these runs, rather than 1.  Divide one full 
		 * run, plus the initial pixel, between the initial and last 
		 * runs
		 */
		initial_pixel_count = (whole_step / 2) + 1;
		final_pixel_count = initial_pixel_count;

		/* 
		 * If the basic run length is even and there's no fractional 
		 * advance, we have 1 pixel that could go to either the 
		 * initial or last partial run, which we'll arbitrarily 
		 * allocate to the last run
		 */
		if ((adj_up == 0) && ((whole_step & 0x01) == 0))
			initial_pixel_count--;

		/* 
		 * If there are an odd number of pixels per run, we have one 
		 * pixel that can't be allocated to either the initial or last 
		 * partial run, so we'll ad 0.5 to the error term so this 
		 * pixel will be handled by the normal rull-run loop
		 */
		if ((whole_step & 0x01) != 0)
			error_term += x_delta;

		/* Draw the first, partial run of pixels */
		__draw_vline(img, &c1, &r1, initial_pixel_count, x_advance, r, g, b);

		/* Draw all full runs */
		for (i = 0; i < (x_delta - 1); i++) {
			/* run is at least this long */
			run_length = whole_step;

			/* 
			 * Advance the error term and add an extra pixel if the
			 * error term so indicates
			 */
			if ((error_term += adj_up) > 0) {
				run_length++;
				/* reset the error term */
				error_term -= adj_down;
			}

			/* Draw this scan line's run */
			__draw_vline(img, &c1, &r1, run_length, x_advance, r, g, b);
		}

		/* Draw the final run of pixels */
		__draw_vline(img, &c1, &r1, final_pixel_count, x_advance, r, g, b);
		return RET_OK;
	}

	return RET_OK;
}

int image_drawtext(IMAGE * image, int r, int c, char *texts, int color)
{
	return text_puts(image, r, c, texts, color);
}

// Draw line: y = k*x + b
int image_drawkxb(IMAGE * image, double k, double b, int color)
{
	int i, j;
	check_image(image);
	for (j = 0; j < image->width; j++) {
		i = (int) (k * j + b);

		if (i >= 0 && i < image->height) {
			image->ie[i][j].r = RGB_R(color);
			image->ie[i][j].g = RGB_G(color);
			image->ie[i][j].b = RGB_B(color);
		}
	}
	return RET_OK;
}

// Peak Signal Noise Ratio
int image_psnr(char oargb, IMAGE * orig, IMAGE * curr, double *psnr)
{
	int i, j, k;
	long long n, sum;
	double d = 0.0;

	check_rgb(oargb);
	check_image(orig);
	check_image(curr);

	if (curr->height != orig->height || curr->width != orig->width) {
		syslog_error("Different size between two images.");
		return RET_ERROR;
	}

	n = sum = 0;
	switch (oargb) {
	case 'R':
		image_foreach(orig, i, j) {
			k = orig->ie[i][j].r - curr->ie[i][j].r;
			n += k * k;
			k = (orig->ie[i][j].r * orig->ie[i][j].r);
			sum += k;
		}
		break;
	case 'G':
		image_foreach(orig, i, j) {
			k = orig->ie[i][j].g - curr->ie[i][j].g;
			n += k * k;
			k = (orig->ie[i][j].g * orig->ie[i][j].g);
			sum += k;
		}
		break;
	case 'B':
		image_foreach(orig, i, j) {
			k = orig->ie[i][j].b - curr->ie[i][j].b;
			n += k * k;
			k = (orig->ie[i][j].b * orig->ie[i][j].b);
			sum += k;
		}
		break;
	default:
		syslog_error("Bad color %c (ARGB).", oargb);
		return RET_ERROR;
		break;
	}

	if (sum == 0)				// Force sum == 1
		sum = 1;
	d = 1.0f * n / sum;

	// Calculate 10 * logf(3 * 255 * 255 * orig->height * orig->width / n);
	if (n == 0)
		n = 1;					// FORCE n == 1
	d = log(65025.0f * orig->height * orig->width / n);
	d = 10.0f * d;
	printf("PSNR = %f\n", d);
	if (psnr)
		*psnr = d;

	return RET_OK;
}

int image_paste(IMAGE * img, int r, int c, IMAGE * small, double alpha)
{
	int i, j, x2, y2, offx, offy;

	check_image(img);
	if (alpha < 0 || alpha > 1.00f) {
		syslog_error("Bad paste alpha parameter %f (0 ~ 1.0f).", alpha);
		return RET_ERROR;
	}
	// Calculate (image & small) region
	offx = offy = 0;
	y2 = r + small->height;
	x2 = c + small->width;
	if (r < 0) {
		r = 0;
		offy = -r;
	}
	if (c < 0) {
		c = 0;
		offx = -c;
	}
	if (y2 > img->height)
		y2 = img->height;
	if (x2 > img->width)
		x2 = img->width;

	// Merge
	for (i = 0; i < y2 - r; i++) {
		for (j = 0; j < x2 - c; j++) {
			img->ie[i + r][j + c].r =
				(BYTE) (alpha * img->ie[i + r][j + c].r + (1.0f - alpha) * small->ie[i + offy][j + offx].r);
			img->ie[i + r][j + c].g =
				(BYTE) (alpha * img->ie[i + r][j + c].g + (1.0f - alpha) * small->ie[i + offy][j + offx].g);
			img->ie[i + r][j + c].b =
				(BYTE) (alpha * img->ie[i + r][j + c].b + (1.0f - alpha) * small->ie[i + offy][j + offx].b);
		}
	}

	return RET_OK;
}

int image_rect_paste(IMAGE * bigimg, RECT * bigrect, IMAGE * smallimg, RECT * smallrect)
{
	int i, j;
	check_image(bigimg);
	check_image(smallimg);

	image_rectclamp(bigimg, bigrect);
	image_rectclamp(smallimg, smallrect);

	if (bigrect->h != smallrect->h || bigrect->w != smallrect->w) {
		syslog_error("Paste size is not same.");
		return RET_ERROR;
	}

	for (i = 0; i < smallrect->h; i++) {
		for (j = 0; j < smallrect->w; j++) {
			bigimg->ie[i + bigrect->r][j + bigrect->c].r = smallimg->ie[i + smallrect->r][j + smallrect->c].r;
			bigimg->ie[i + bigrect->r][j + bigrect->c].g = smallimg->ie[i + smallrect->r][j + smallrect->c].g;
			bigimg->ie[i + bigrect->r][j + bigrect->c].b = smallimg->ie[i + smallrect->r][j + smallrect->c].b;
		}
	}
	return RET_OK;
}

int image_make_noise(IMAGE * img, char orgb, int rate)
{
	int i, j, v;

	check_argb(orgb);
	check_image(img);

	srandom((unsigned int) time(NULL));
	image_foreach(img, i, j) {
		if (i == 0 || i == img->height - 1 || j == 0 || j == img->width - 1)
			continue;

		v = random() % 100;
		if (v >= rate)
			continue;
		v = (v % 2 == 0) ? 0 : 255;
		// Add noise
		image_setvalue(img, orgb, i, j, (BYTE) v);
	}

	return RET_OK;
}

// Auto middle value filter
int image_delete_noise(IMAGE * img)
{
	int i, j, index;
	RGBA_8888 *mid, *cell;
	IMAGE *orig;

	check_image(img);
	orig = image_copy(img);
	check_image(orig);
	for (i = 1; i < img->height - 1; i++) {
		for (j = 1; j < img->width - 1; j++) {
			__nb3x3_map(orig, i, j);
			cell = &(img->base[i * img->width + j]);
			index = __color_rgbfind(cell, 3 * 3, __image_rgb_nb);

			if (ABS(index - 4) > 1) {
				mid = __image_rgb_nb[4];
				cell->r = mid->r;
				cell->g = mid->g;
				cell->b = mid->b;
			}
		}
	}
	image_destroy(orig);

	return RET_OK;
}

int image_statistics(IMAGE * img, char orgb, double *avg, double *stdv)
{
	RECT rect;

	image_rect(&rect, img);
	return image_rect_statistics(img, &rect, orgb, avg, stdv);
}


int image_rect_statistics(IMAGE * img, RECT * rect, char orgb, double *avg, double *stdv)
{
	BYTE n;
	int i, j;
	double davg, dstdv;

	check_image(img);

	image_rectclamp(img, rect);

	davg = dstdv = 0.0f;
	switch (orgb) {
	case 'A':
		rect_foreach(rect, i, j) {
			color_rgb2gray(img->ie[rect->r + i][rect->c + j].r,
						   img->ie[rect->r + i][rect->c + j].g, img->ie[rect->r + i][rect->c + j].b, &n);
			davg += n;
			dstdv += n * n;
		}
		break;
	case 'R':
		rect_foreach(rect, i, j) {
			n = img->ie[rect->r + i][rect->c + j].r;
			davg += n;
			dstdv += n * n;
		}

		break;
	case 'G':
		rect_foreach(rect, i, j) {
			n = img->ie[rect->r + i][rect->c + j].g;
			davg += n;
			dstdv += n * n;
		}

		break;
	case 'B':
		rect_foreach(rect, i, j) {
			n = img->ie[rect->r + i][rect->c + j].b;
			davg += n;
			dstdv += n * n;
		}

		break;
	default:
		break;
	}

	davg /= (rect->h * rect->w);
	*avg = davg;
	*stdv = sqrt(dstdv / (rect->h * rect->w) - davg * davg);

	return RET_OK;
}


IMAGE *image_hmerge(IMAGE * image1, IMAGE * image2)
{
	int i, j, k;
	IMAGE *image;

	CHECK_IMAGE(image1);
	CHECK_IMAGE(image2);

	image = image_create(MAX(image1->height, image2->height), image1->width + image2->width);
	CHECK_IMAGE(image);
	// paste image 1
	image_foreach(image1, i, j) {
		image->ie[i][j].r = image1->ie[i][j].r;
		image->ie[i][j].g = image1->ie[i][j].g;
		image->ie[i][j].b = image1->ie[i][j].b;
	}

	// paste image 2
	k = image1->width;
	image_foreach(image2, i, j) {
		image->ie[i][j + k].r = image2->ie[i][j].r;
		image->ie[i][j + k].g = image2->ie[i][j].g;
		image->ie[i][j + k].b = image2->ie[i][j].b;
	}

	return image;
}

// matter center
int image_mcenter(IMAGE * img, char orgb, int *crow, int *ccol)
{
	RECT rect;
	image_rect(&rect, img);
	return image_rect_mcenter(img, &rect, orgb, crow, ccol);
}

// matter center
int image_rect_mcenter(IMAGE * img, RECT * rect, char orgb, int *crow, int *ccol)
{
	BYTE n;
	int i, j;
	long long m00, m01, m10;

	check_argb(orgb);
	check_image(img);
	image_rectclamp(img, rect);

	if (!crow || !ccol) {
		syslog_error("Result is NULL.");
		return RET_ERROR;
	}

	m00 = m10 = m01 = 0;
	switch (orgb) {
	case 'A':
		rect_foreach(rect, i, j) {
			color_rgb2gray(img->ie[i + rect->r][j + rect->c].r,
						   img->ie[i + rect->r][j + rect->c].g, img->ie[i + rect->r][j + rect->c].b, &n);
			m00 += n;
			m10 += i * n;
			m01 += j * n;
		}
		break;
	case 'R':
		rect_foreach(rect, i, j) {
			n = img->ie[i + rect->r][j + rect->c].r;
			m00 += n;
			m10 += i * n;
			m01 += j * n;
		}
		break;
	case 'G':
		rect_foreach(rect, i, j) {
			n = img->ie[i + rect->r][j + rect->c].g;
			m00 += n;
			m10 += i * n;
			m01 += j * n;
		}
		break;
	case 'B':
		rect_foreach(rect, i, j) {
			n = img->ie[i + rect->r][j + rect->c].b;
			m00 += n;
			m10 += i * n;
			m01 += j * n;
		}
		break;
	default:
		break;
	}

	if (m00 == 0) {
		*crow = rect->r + rect->h / 2;
		*ccol = rect->c + rect->w / 2;
		return RET_OK;
	}
	*crow = (int) (m10 / m00 + rect->r + rect->h / 2);
	*ccol = (int) (m01 / m00 + rect->c + rect->w / 2);

	return RET_OK;
}


IMAGE *image_subimg(IMAGE * img, RECT * rect)
{
	int i;
	IMAGE *sub;

	image_rectclamp(img, rect);
	sub = image_create(rect->h, rect->w);
	CHECK_IMAGE(sub);
#if 0
	int j;
	for (i = 0; i < rect->h; i++) {
		for (j = 0; j < rect->w; j++) {
			sub->ie[i][j].r = img->ie[i + rect->r][j + rect->c].r;
			sub->ie[i][j].g = img->ie[i + rect->r][j + rect->c].g;
			sub->ie[i][j].b = img->ie[i + rect->r][j + rect->c].b;
		}
	}
#else							// Fast
	for (i = 0; i < rect->h; i++) {
		memcpy(&(sub->ie[i][0]), &(img->ie[i + rect->r][rect->c]), sizeof(RGBA_8888) * rect->w);
	}
#endif
	return sub;
}

// Gray stattics
MATRIX *image_gstatics(IMAGE * img, int rows, int cols)
{
	BYTE gray;
	MATRIX *mat;
	int i, j, bh, bw, ball, i1, j1;

	mat = matrix_create(rows, cols);
	CHECK_MATRIX(mat);

	bh = (img->height + rows - 1) / rows;
	bw = (img->width + cols - 1) / cols;
	image_foreach(img, i, j) {
		i1 = (i / bh);
		j1 = (j / bw);
		color_rgb2gray(img->ie[i][j].r, img->ie[i][j].g, img->ie[i][j].b, &gray);
		mat->me[i1][j1] += gray;
	}
	ball = bh * bw;
	matrix_foreach(mat, i, j)
		mat->me[i][j] /= ball;

	return mat;
}

// Example: image_clahe(image, 4, 4, 4);
int image_clahe(IMAGE * image, int grid_rows, int grid_cols, double limit)
{
	BYTE g;
	int i, j, h, w, i2, j2;
	double u, v, d;
	int d1r, d1c, d2r, d2c, d3r, d3c, d4r, d4c;
	RECT rect;
	HISTOGRAM hist[CLAHE_MAX_ROWS][CLAHE_MAX_COLS], *d1, *d2, *d3, *d4;

	check_image(image);

	if (grid_rows > CLAHE_MAX_ROWS)
		grid_rows = CLAHE_MAX_ROWS;
	if (grid_cols > CLAHE_MAX_COLS)
		grid_cols = CLAHE_MAX_COLS;

	h = (image->height + grid_rows - 1) / grid_rows;
	w = (image->width + grid_cols - 1) / grid_cols;
	limit = MAX(1, (limit * h * w / 256.0));

	color_togray(image);
	for (i = 0; i < grid_rows; i++) {
		for (j = 0; j < grid_cols; j++) {
			rect.r = i * h;
			rect.h = h;
			rect.c = j * w;
			rect.w = w;
			image_rectclamp(image, &rect);

			histogram_rect(&hist[i][j], image, &rect);	// Suppose: image is gray       
			histogram_clip(&hist[i][j], (int) limit);
			histogram_cdf(&hist[i][j]);
			histogram_map(&hist[i][j], 255);
			// histogram_dump(&hist[i][j]);
		}
	}

	// Interpolate
	/*************************************************************************************
	d1    d2
	    (p)
	d3    d4
	(1.0 - u) * (1.0 - v) * d1 + (1.0 - u)*v*d2 + u*(1.0 - v)*d3 + u*v*d4
	**************************************************************************************/
	for (i = 0; i <= grid_rows; i++) {
		if (i == 0) {
			rect.r = 0;
			rect.h = h / 2;

			d1r = d2r = d3r = d4r = 0;
		} else if (i == grid_rows) {
			rect.r = (i - 1) * h + h / 2;
			rect.h = h / 2;

			d1r = d2r = d3r = d4r = grid_rows - 1;
		} else {
			rect.r = (i - 1) * h + h / 2;
			rect.h = h;

			d1r = d2r = i - 1;
			d3r = d4r = i;
		}

		for (j = 0; j <= grid_cols; j++) {
			if (j == 0) {
				rect.c = 0;
				rect.w = w / 2;

				d1c = d2c = d3c = d4c = 0;
			} else if (j == grid_cols) {
				rect.c = (j - 1) * w + w / 2;
				rect.w = w / 2;

				d1c = d2c = d3c = d4c = grid_cols - 1;
			} else {
				rect.c = (j - 1) * w + w / 2;
				rect.w = w;

				d1c = d3c = j - 1;
				d2c = d4c = j;
			}

			d1 = &hist[d1r][d1c];
			d2 = &hist[d2r][d2c];
			d3 = &hist[d3r][d3c];
			d4 = &hist[d4r][d4c];

			for (i2 = rect.r; i2 < rect.r + rect.h; i2++) {
				u = (double) (i2 - rect.r) / (double) rect.h;

				for (j2 = rect.c; j2 < rect.c + rect.w; j2++) {
					v = (double) (j2 - rect.c) / (double) rect.w;

					g = image->ie[i2][j2].r;
					d = (1.0 - u) * (1.0 - v) * d1->map[g] + (1.0 - u) * v * d2->map[g] + u * (1.0 - v) * d3->map[g] +
						u * v * d4->map[g];

					g = (d > 255) ? 255 : (int) d;

					image->ie[i2][j2].r = g;
					image->ie[i2][j2].g = g;
					image->ie[i2][j2].b = g;
				}
			}
		}
	}

	return RET_OK;
}

int image_niblack(IMAGE * image, int radius, double scale)
{
	int i, j;
	MATRIX *mean, *stdv, *temp, *area;

	check_image(image);

	// Fast count 
	area = matrix_create(image->height, image->width);
	check_matrix(area);
	matrix_pattern(area, "one");

	temp = matrix_copy(area);
	matrix_integrate(temp);
	matrix_foreach(area, i, j) {
		area->me[i][j] = matrix_difference(temp, i - radius, j - radius, i + radius, j + radius);
	}
	matrix_destroy(temp);

	color_togray(image);
	mean = image_getplane(image, 'R');
	stdv = matrix_copy(mean);

	// mean
	temp = matrix_copy(mean);
	matrix_integrate(temp);
	matrix_foreach(mean, i, j) {
		mean->me[i][j] = matrix_difference(temp, i - radius, j - radius, i + radius, j + radius);
		mean->me[i][j] /= area->me[i][j];
	}
	matrix_destroy(temp);

	// stdv
	matrix_foreach(stdv, i, j)
		stdv->me[i][j] = stdv->me[i][j] * stdv->me[i][j];
	temp = matrix_copy(stdv);
	matrix_integrate(temp);
	matrix_foreach(stdv, i, j) {
		stdv->me[i][j] = matrix_difference(temp, i - radius, j - radius, i + radius, j + radius);
		stdv->me[i][j] /= area->me[i][j];

		stdv->me[i][j] -= mean->me[i][j] * mean->me[i][j];
		stdv->me[i][j] = sqrt(stdv->me[i][j]);
	}
	matrix_destroy(temp);

	// Threshold
	matrix_foreach(stdv, i, j) {
		stdv->me[i][j] *= scale;
		stdv->me[i][j] += mean->me[i][j];
	}

	image_foreach(image, i, j) {
		if (image->ie[i][j].r >= stdv->me[i][j]) {
			image->ie[i][j].r = 255;
			image->ie[i][j].g = 255;
			image->ie[i][j].b = 255;
		} else {
			image->ie[i][j].r = 0;
			image->ie[i][j].g = 0;
			image->ie[i][j].b = 0;
		}
	}

	matrix_destroy(mean);
	matrix_destroy(stdv);
	matrix_destroy(area);

	return RET_OK;
}

int image_negative(IMAGE * image)
{
	int i, j;
	check_image(image);

	image_foreach(image, i, j) {
		image->ie[i][j].r = 255 - image->ie[i][j].r;
		image->ie[i][j].g = 255 - image->ie[i][j].g;
		image->ie[i][j].b = 255 - image->ie[i][j].b;
	}

	return RET_OK;
}

int image_show(IMAGE * image, char *title)
{
	char str[256];

	check_image(image);

	snprintf(str, sizeof(str) - 1, "/tmp/%s.jpg", title);
	image_save(image, str);
	snprintf(str, sizeof(str) - 1, "display /tmp/%s.jpg", title);

	return system(str);
}

IMAGE *image_from_tensor(TENSOR * tensor, int k)
{
	int i, j;
	IMAGE *image;
	BYTE *R, *G, *B, *A;

	CHECK_TENSOR(tensor);

	if (k < 0 || k >= tensor->batch) {
		syslog_error("image index over tensor batch size.");
		return NULL;
	}

	image = image_create(tensor->height, tensor->width);
	CHECK_IMAGE(image);
	R = tensor->base + k * (tensor->chan * tensor->height * tensor->width);
	G = R + tensor->height * tensor->width;
	B = G + tensor->height * tensor->width;
	A = B + tensor->height * tensor->width;

	image_foreach(image, i, j) {
		image->ie[i][j].r = *R++;
		image->ie[i][j].g = *G++;
		image->ie[i][j].b = *B++;
		image->ie[i][j].a = *A++;
	}

	return image;
}

TENSOR *tensor_from_image(IMAGE *image)
{
	int i, j;
	TENSOR *tensor;
	BYTE *R, *G, *B, *A;

	CHECK_IMAGE(image);

	tensor = tensor_create(1, sizeof(RGBA_8888), image->height, image->width);
	CHECK_TENSOR(tensor);

	R = tensor->base;
	G = R + tensor->height * tensor->width;
	B = G + tensor->height * tensor->width;
	A = B + tensor->height * tensor->width;
	image_foreach(image, i, j) {
		*R++ = image->ie[i][j].r;
		*G++ = image->ie[i][j].g;
		*B++ = image->ie[i][j].b;
		*A++ = image->ie[i][j].a;
	}
	
	return tensor;
}


IMAGE *image_fromab(BYTE * buf)
{
	IMAGE *image;
	AbHead abhead;

	if (abhead_decode(buf, &abhead) != RET_OK) {
		syslog_error("Bad Ab Head.");
		return NULL;
	}

	image = image_create(abhead.h, abhead.w);
	CHECK_IMAGE(image);
	image->opc = abhead.opc;
	memcpy(image->base, buf + sizeof(AbHead), abhead.len);

	return image;
}

BYTE *image_toab(IMAGE * image)
{
	BYTE *buf;
	int data_size;

	CHECK_IMAGE(image);

	data_size = image->height * image->width * sizeof(RGBA_8888);
	buf = (BYTE *) malloc(sizeof(AbHead) + data_size);
	if (!buf) {
		syslog_error("Allocate memory.");
		return NULL;
	}

	image_abhead(image, buf);
	memcpy(buf + sizeof(AbHead), image->base, data_size);

	return buf;
}

#if 0
IMAGE *image_read(int fd)
{
	AbHead abhead;
	IMAGE *image;
	ssize_t n, data_size;
	BYTE headbuf[sizeof(AbHead)], *databuf;

	if (read(fd, headbuf, sizeof(AbHead)) != sizeof(AbHead)) {
		syslog_error("Reading array buffer head.\n");
		return NULL;
	}
	// 1. Get abhead ?
	if (abhead_decode(headbuf, &abhead) != RET_OK) {
		syslog_error("Bad AbHead: t = %c%c, len = %d, crc = %x .\n", abhead.t[0], abhead.t[1], abhead.len, abhead.crc);
		while (read(fd, headbuf, sizeof(headbuf)) > 0);	// Skip left dirty data ...

		return NULL;
	}
	// 2. Get image data
	image = image_create(abhead.h, abhead.w);
	CHECK_IMAGE(image);
	image->opc = abhead.opc;	// Save RPC Method
	data_size = image->height * image->width * sizeof(RGBA_8888);
	databuf = (BYTE *) image->base;
	n = 0;
	while (n < data_size) {
		n += read(fd, databuf + n, data_size - n);
		if (n <= 0)
			break;
	}
	if (n != data_size) {
		syslog_error("Received %ld bytes image data -- expect %ld !!!\n", n, data_size);
		image_destroy(image);
		return NULL;
	}

	return image;
}

int image_write(int fd, IMAGE * image)
{
	ssize_t data_size;
	BYTE buffer[sizeof(AbHead)];

	check_image(image);

	data_size = image->height * image->width * sizeof(RGBA_8888);

	// 1. encode abhead and send
	if (image_abhead(image, buffer) != RET_OK) {
		syslog_error("Image AbHead encode.");
	}
	if (write(fd, buffer, sizeof(AbHead)) != sizeof(AbHead)) {
		syslog_error("Write AbHead.");
		return RET_ERROR;
	}
	// 2. send data
	if (write(fd, image->base, data_size) != data_size) {
		syslog_error("Write image data.");
		return RET_ERROR;
	}

	return RET_OK;
}
#endif

int image_abhead(IMAGE * image, BYTE * buffer)
{
	AbHead t;
	check_image(image);

	// encode abhead
	abhead_init(&t);
	t.len = image->height * image->width * sizeof(RGBA_8888);
	t.b = 1;
	t.c = sizeof(RGBA_8888);
	t.h = image->height;
	t.w = image->width;
	t.opc = image->opc;

	return abhead_encode(&t, buffer);
}


#ifdef CONFIG_NNG

int image_send(nng_socket socket, IMAGE * image)
{
	int ret;
	nng_msg *msg = NULL;
	BYTE head_buf[sizeof(AbHead)];
	size_t send_size;

	check_image(image);
	image_abhead(image, head_buf);
	if ((ret = nng_msg_alloc(&msg, 0)) != 0) {
		syslog_error("nng_msg_alloc: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}
	if ((ret = nng_msg_append(msg, head_buf, sizeof(AbHead))) != 0) {
		syslog_error("nng_msg_append: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}

	send_size = image->height * image->width * sizeof(RGBA_8888);
	if ((ret = nng_msg_append(msg, image->base, send_size)) != 0) {
		syslog_error("nng_msg_append: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}
	if ((ret = nng_sendmsg(socket, msg, NNG_FLAG_ALLOC)) != 0) {
		syslog_error("nng_sendmsg: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}
	// nng_msg_free(msg); // NNG_FLAG_ALLOC will "call nng_msg_free auto"

	return RET_OK;
}

IMAGE *image_recv(nng_socket socket)
{
	int ret;
	BYTE *recv_buf = NULL;
	size_t recv_size;
	IMAGE *recv_image = NULL;

	if ((ret = nng_recv(socket, &recv_buf, &recv_size, NNG_FLAG_ALLOC)) != 0) {
		syslog_error("nng_recv: return code = %d, message = %s", ret, nng_strerror(ret));
		nng_free(recv_buf, recv_size);	// Bad message received...
		return NULL;
	}

	if (valid_ab(recv_buf, recv_size))
		recv_image = image_fromab(recv_buf);

	nng_free(recv_buf, recv_size);	// Data has been saved ...

	return recv_image;
}
#endif

