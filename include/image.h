
/************************************************************************************
***
***	Copyright 2010-2020 Dell Du(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Sat Jul 31 14:19:59 HKT 2010
***
************************************************************************************/


#ifndef __IMAGE_H
#define __IMAGE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "common.h"
#include "matrix.h"
#include "vector.h"
#include "tensor.h"
#include "nnmsg.h"

#define IMAGE_VERSION "1.0.0"

	typedef struct {
		BYTE r, g, b, a;
	} RGBA_8888;
#define RGB_R(x) ((BYTE)((x) >> 16) & 0xff)
#define RGB_G(x) ((BYTE)((x) >> 8) & 0xff)
#define RGB_B(x) ((BYTE)((x) & 0xff))
#define RGB_INT(r, g, b) ((r) << 16 | (g) << 8 | (b))

	typedef struct {
		float L, a, b;
	} LAB;

	typedef struct {
		float L, u, v;
	} Luv;

	typedef struct {
		DWORD magic;			// IMAGE_MAGIC
		WORD height, width, format;
		RGBA_8888 **ie, *base;

		// Extentend for cluster & color mask
		int K, KColors[256], KRadius, KInstance;
	} IMAGE;

#define IMAGE_RGBA 0
#define IMAGE_GRAY 1
#define IMAGE_RGB565 2
#define IMAGE_MASK 3

#define image_foreach(img, i, j) \
	for (i = 0; i < img->height; i++) \
		for (j = 0; j < img->width; j++)

#define check_image(img) \
	do { \
		if (! image_valid(img)) { \
			syslog_error("Bad image.\n"); \
			return RET_ERROR; \
		} \
	} while(0)

#define CHECK_IMAGE(img) \
	do { \
		if (! image_valid(img)) { \
			syslog_error("Bad image.\n"); \
			return NULL; \
		} \
	} while(0)


#define image_rect(rect, img) \
	do { (rect)->r = 0; (rect)->c = 0; (rect)->h = img->height; (rect)->w = img->width; } while(0)


#define check_rgb(orgb) \
	do { \
		if (! strchr("ARGB", orgb)) { \
			syslog_error("Bad color %c (ARGB).\n", orgb); \
			return RET_ERROR; \
		} \
	} while (0)

#define check_argb(orgb) \
	do { \
		if (! strchr("ARGB", orgb)) { \
			syslog_error("Bad color %c (ARGB).\n", orgb); \
			return RET_ERROR; \
		} \
	} while (0)

#define CHECK_RGB(orgb) \
	do { \
		if (! strchr("RGB", orgb)) { \
			syslog_error("Bad color %c (RGB).\n", orgb); \
			return NULL; \
		} \
	} while (0)

#define CHECK_ARGB(orgb) \
	do { \
		if (! strchr("ARGB", orgb)) { \
			syslog_error("Bad color %c (ARGB).\n", orgb); \
			return NULL; \
		} \
	} while (0)

#define CHECK_ARGBH(orgb) \
	do { \
		if (! strchr("ARGBH", orgb)) { \
			syslog_error("Bad color %c (ARGBH).", orgb); \
			return NULL; \
		} \
	} while (0)

#define RGB565_R(x) ((((x) >> 11) & 0x1f) << 3)
#define RGB565_G(x) ((((x) >> 5) & 0x3f) << 2)
#define RGB565_B(x) (((x) & 0x1f) << 3)
#define RGB565_NO(r, g, b) (((r)&0xf8) << 8 | ((g) & 0xfc) << 3 | ((b) & 0xf8) >> 3)

	IMAGE *image_create(WORD h, WORD w);
	IMAGE *image_load(char *fname);
	IMAGE *image_copy(IMAGE * img);
	IMAGE *image_zoom(IMAGE * img, int nh, int nw, int method);
	IMAGE *image_hmerge(IMAGE * image1, IMAGE * image2);

	int image_valid(IMAGE * img);
	int image_clear(IMAGE * img);
	int image_outdoor(IMAGE * img, int i, int di, int j, int dj);
	int image_rectclamp(IMAGE * img, RECT * rect);
	int image_save(IMAGE * img, const char *fname);

	int image_show(IMAGE * image, char *title);

	BYTE image_getvalue(IMAGE * img, char oargb, int r, int c);
	void image_setvalue(IMAGE * img, char oargb, int r, int c, BYTE x);
	MATRIX *image_getplane(IMAGE * img, char oargb);
	MATRIX *image_rect_plane(IMAGE * img, char oargb, RECT * rect);

	int image_setplane(IMAGE * img, char oargb, MATRIX * mat);

	void color_rgb2gray(BYTE r, BYTE g, BYTE b, BYTE * gray);
	int skin_detect(IMAGE * img);
	int skin_statics(IMAGE * img, RECT * rect);

	int color_cluster(IMAGE * image, int num);
	int color_picker();
	int color_balance(IMAGE * img, int method, int debug);

	int color_correct(IMAGE * img, float gain_r, float gain_g, float gain_b);
	int color_togray(IMAGE * img);
	int color_torgb565(IMAGE * img);

	void color_rgb2lab(BYTE R, BYTE G, BYTE B, float *L, float *a, float *b);
	void color_lab2rgb(float L, float a, float b, BYTE *R, BYTE *G, BYTE *B);
	float color_distance(RGBA_8888 * c1, RGBA_8888 * c2);

	int image_drawline(IMAGE * img, int r1, int c1, int r2, int c2, int color);
	int image_drawrect(IMAGE * img, RECT * rect, int color, int fill);
	int image_drawtext(IMAGE * image, int r, int c, char *texts, int color);
	int image_drawkxb(IMAGE * image, float k, float b, int color);

	int image_psnr(char oargb, IMAGE * orig, IMAGE * now, float *psnr);
	int image_paste(IMAGE * img, int r, int c, IMAGE * small, float alpha);
	int image_rect_paste(IMAGE * bigimg, RECT * bigrect, IMAGE * smallimg, RECT * smallrect);

	int image_statistics(IMAGE * img, char orgb, float *avg, float *stdv);
	int image_rect_statistics(IMAGE * img, RECT * rect, char orgb, float *avg, float *stdv);
	void image_destroy(IMAGE * img);

// Image Color ...
	int color_midval(IMAGE * img, char orgb);
	VECTOR *color_vector(IMAGE * img, RECT * rect, int ndim);
	float color_likeness(IMAGE * f, IMAGE * g, RECT * rect, int ndim);

// Image Shape ...
	VECTOR *shape_vector(IMAGE * img, RECT * rect, int ndim);
	float shape_likeness(IMAGE * f, IMAGE * g, RECT * rect, int ndim);
// Image Texture ...
	VECTOR *texture_vector(IMAGE * img, RECT * rect, int ndim);
	float texture_likeness(IMAGE * f, IMAGE * g, RECT * rect, int ndim);
// Contour & Skeleton
	int image_contour(IMAGE * img);
	int image_skeleton(IMAGE * img);
// Middle value edge &  Canny edge
	int shape_midedge(IMAGE * img);
	int shape_bestedge(IMAGE * img);

// Hough Transform
	int line_detect(IMAGE * img, int debug);
	int line_lsm(IMAGE * image, RECT * rect, float *k, float *b, int debug);

	int motion_updatebg(IMAGE * A, IMAGE * B, IMAGE * C, IMAGE * bg);
	int motion_detect(IMAGE * fg, IMAGE * bg, int debug);
	int object_fast_detect(IMAGE * fg);

// Matter center
	int image_mcenter(IMAGE * img, char orgb, int *crow, int *ccol);
	int image_rect_mcenter(IMAGE * img, RECT * rect, char orgb, int *crow, int *ccol);

	IMAGE *image_subimg(IMAGE * img, RECT * rect);

	void image_drawrects(IMAGE * img);
	MATRIX *image_gstatics(IMAGE * img, int rows, int cols);

// Blending 
	int image_blend(IMAGE * src, IMAGE * mask, IMAGE * dst, int top, int left, int debug);

// Seaming
	int *image_seampath(IMAGE * image_a, RECT * rect_a, IMAGE * image_b, RECT * rect_b, int mode);
	IMAGE *image_seammask(IMAGE * image_a, RECT * rect_a, IMAGE * image_b, RECT * rect_b, int mode);

// Retinex
	int image_retinex(IMAGE * image, int nscale);

	int image_negative(IMAGE * image);
	int image_clahe(IMAGE * image, int grid_rows, int grid_cols, float limit);
	int image_niblack(IMAGE * image, int radius, float scale);

// Filter
	int image_make_noise(IMAGE * img, char orgb, int rate);
	int image_delete_noise(IMAGE * img);
	int image_gauss_filter(IMAGE * image, float sigma);
	int image_guided_filter(IMAGE * img, IMAGE * guidance, int radius, float eps, int scale, int debug);
	int image_beeps_filter(IMAGE * img, float stdv, float dec, int debug);
	int image_lee_filter(IMAGE * img, int radius, float eps, int debug);
	int image_dehaze_filter(IMAGE * img, int radius, int debug);
	int image_medium_filter(IMAGE * img, int radius);
	int image_fast_filter(IMAGE * img, int n, int *kernel, int total);
	int image_gauss3x3_filter(IMAGE * img, RECT * rect);
	int image_gauss5x5_filter(IMAGE * img, RECT * rect);
	int image_dot_filter(IMAGE * img, int i, int j);
	int image_rect_filter(IMAGE * img, RECT * rect, int n, int *kernel, int total);

// Hash
	HASH64 image_ahash(IMAGE * image, char oargb, RECT * rect);
	HASH64 image_phash(IMAGE * image, char oargb, RECT * rect);
	HASH64 shape_hash(IMAGE * image, RECT * rect);
	HASH64 texture_hash(IMAGE * image, RECT * rect);
	int hash_hamming(HASH64 f1, HASH64 f2);
	float hash_likeness(HASH64 f1, HASH64 f2);
	void hash_dump(char *title, HASH64 finger);

// Histogram
#define HISTOGRAM_MAX_COUNT 256
	typedef struct {
		int total;
		int count[HISTOGRAM_MAX_COUNT];

		float cdf[HISTOGRAM_MAX_COUNT];

		int map[HISTOGRAM_MAX_COUNT];
	} HISTOGRAM;

	void histogram_reset(HISTOGRAM * h);
	void histogram_add(HISTOGRAM * h, int c);
	void histogram_del(HISTOGRAM * h, int c);
	int histogram_middle(HISTOGRAM * h);
	int histogram_top(HISTOGRAM * h, float ratio);
	int histogram_clip(HISTOGRAM * h, int threshold);
	int histogram_cdf(HISTOGRAM * h);
	int histogram_map(HISTOGRAM * h, int max);
	float histogram_likeness(HISTOGRAM * h1, HISTOGRAM * h2);
	void histogram_sum(HISTOGRAM * sum, HISTOGRAM * sub);
	int histogram_rect(HISTOGRAM * hist, IMAGE * img, RECT * rect);
	void histogram_dump(HISTOGRAM * h);

// Mask is all you need
#define MASK IMAGE

	int color_instance(MASK * image, int KRadius);
	int mask_show();

// Tensor
	IMAGE *image_from_tensor(TENSOR * tensor, int k);
	TENSOR *tensor_from_image(IMAGE * image, int with_alpha);
#if defined(__cplusplus)
}
#endif
#endif							// __IMAGE_H
