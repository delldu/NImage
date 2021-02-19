
/************************************************************************************
***
***	Copyright 2010-2020 Dell Du(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Sat Jul 31 14:19:59 HKT 2010
***
************************************************************************************/


#include <math.h>
#include "image.h"

// CIE 1931:
// C:   98.07, 100.00, 118.22
// D50: 96.42, 100.00,  82.51
// D65: 95.04, 100.00, 108.88
// CIE 1964
// C:   97.29, 100.00, 116.14
// D50: 96.72, 100.00,  81.43
// D65: 94.81, 100.00, 107.32

// Follow CIE 1931, D65
// #define  LAB_X_R  0.433953f		/* = xyzXr / 0.950456 */
// #define  LAB_X_G  0.376219f		/* = xyzXg / 0.950456 */
// #define  LAB_X_B  0.189828f		/* = xyzXb / 0.950456 */

// #define  LAB_Y_R  0.212671f		/* = xyzYr */
// #define  LAB_Y_G  0.715160f		/* = xyzYg */
// #define  LAB_Y_B  0.072169f		/* = xyzYb */

// #define  LAB_Z_R  0.017758f		/* = xyzZr / 1.088754 */
// #define  LAB_Z_G  0.109477f		/* = xyzZg / 1.088754 */
// #define  LAB_Z_B  0.872766f		/* = xyzZb / 1.088754 */

#define CUBE_CELL_OFFSET(r,c,d) (((r)*cols + (c))*(levs) + (d))

// Color balance
#define COLOR_BALANCE_GRAY_WORLD 0
#define COLOR_BALANCE_FULL_REFLECT 1

/*
From Keith Jack's excellent book "Video Demystified" (ISBN 1-878707-09-4)
Y = 0.257R + 0.504G + 0.098B + 16
U = 0.148R - 0.291G + 0.439B + 128
V = 0.439R - 0.368G - 0.071B + 128

R = 1.164(Y - 16) + 1.596(V - 128)
G = 1.164(Y - 16) - 0.391(U - 128) - 0.813(V - 128)
B = 1.164(Y - 16) + 2.018(U - 128)
*/

#define RGB_TO_YCBCR( r, g, b, y, cb, cr )       \
do {                                                                  \
	int _r = (r), _g = (g), _b = (b);                    \
	                                                          \
	(y) = (66 * _r + 129 * _g + 25 * _b  +  16*256 + 128) >> 8; \
	(cb) = (-38 * _r  - 74 * _g + 112 * _b  + 128*256 + 128) >> 8; \
	(cr) = (112 * _r - 94 * _g  -18 * _b  + 128*256 + 128) >> 8; \
} while (0)

// xxxx, how to use them ?
extern int color_rgbcmp(RGBA_8888 * c1, RGBA_8888 * c2);
extern void color_rgb2ycbcr(BYTE R, BYTE G, BYTE B, BYTE * y, BYTE * cb, BYTE * cr);
extern int color_prmgain(IMAGE * img, float *r_gain, float *g_gain, float *b_gain);
extern int *color_count(IMAGE * image, int rows, int cols, int levs);

static int __color_rgbcmp(const void *p1, const void *p2)
{
	RGBA_8888 *c1 = *(RGBA_8888 * const *) p1;
	RGBA_8888 *c2 = *(RGBA_8888 * const *) p2;
	return color_rgbcmp(c1, c2);
}

// Color is skin ? Maybe !!!
int color_beskin(BYTE r, BYTE g, BYTE b)
{
	BYTE y, cb, cr;

	color_rgb2ycbcr(r, g, b, &y, &cb, &cr);
	if (y < 64 || y > 235)
		return 0;

	// Cb in [77, 127], Cr in [133, 173]
	if (cb >= 77 && cb <= 127 && cr >= 133 && cr <= 173)
		return 1;

	// cb in [80, 135], cr in [136, 177]
	if (cb >= 80 && cb <= 135 && cr >= 136 && cr <= 177)
		return 1;

	return 0;
}

// RGB color space
int color_rgbcmp(RGBA_8888 * c1, RGBA_8888 * c2)
{
	if (c1->g != c2->g)
		return (c1->g - c2->g);
	if (c1->r != c2->r)
		return (c1->r - c2->r);
	return (c1->b - c2->b);
}

// CIELAB values range as follows: 
// L lies between 0 and 100, and a and b lie between -110 and 110. 
// which is the D65 standard.

// TENSOR *tensor_rgb2lab(IMAGE *image);
// TENSOR *tensor_lab2rgb(TENSOR *tenosr);
// Drawable --> `
void color_rgb2lab(BYTE R, BYTE G, BYTE B, float *L, float *a, float *b)
{
	float r0, g0, b0, x, y, z;

	r0 = (float)R/255.f; g0 = (float)G/255.f; b0 = (float)B/255.f;

	r0 = (r0 > 0.04045) ? powf((r0 + 0.055) / 1.055, 2.4) : r0 / 12.92;
	g0 = (g0 > 0.04045) ? powf((g0 + 0.055) / 1.055, 2.4) : g0 / 12.92;
	b0 = (b0 > 0.04045) ? powf((b0 + 0.055) / 1.055, 2.4) : b0 / 12.92;

	x = (r0 * 0.412453 + g0 * 0.357680 + b0 * 0.180423) / 0.95047;
	y = (r0 * 0.212671 + g0 * 0.715160 + b0 * 0.072169) / 1.00000;
	z = (r0 * 0.019334 + g0 * 0.119193 + b0 * 0.950227) / 1.08883;

	// 0.137931 = 16.f/116.f
	x = (x > 0.008856) ? powf(x, 0.333333f) : (7.787f * x) + 0.137931;
	y = (y > 0.008856) ? powf(y, 0.333333f) : (7.787f * y) + 0.137931;
	z = (z > 0.008856) ? powf(z, 0.333333f) : (7.787f * z) + 0.137931;

	*L = 116.f * y - 16.f;
	*a = 500.f * (x - y);
	*b = 200.f * (y - z);
}

void color_lab2rgb(float L, float a, float b, BYTE *R, BYTE *G, BYTE *B)
{
	float r0, g0, b0, x, y, z;

	y = (L + 16.f) / 116.f;
	x = a / 500.f + y;
	z = y - b / 200.f;

	// 0.137931 = 16/116
	// x * x * x > 0.008856 <==> x > 0.206893
	x = 0.95047 * ((x > 0.206893) ? x * x * x : (x - 0.137931) / 7.787);
	y = 1.00000 * ((y > 0.206893) ? y * y * y : (y - 0.137931) / 7.787);
	z = 1.08883 * ((z > 0.206893) ? z * z * z : (z - 0.137931) / 7.787);

	r0 = x *  3.24048 + y * -1.53715 + z * -0.49853;
	g0 = x * -0.96925 + y *  1.87599 + z *  0.04155;
	b0 = x *  0.05564 + y * -0.20404 + z *  1.05731;

	// 0.416666 == 1.f/2.4f
	r0 = (r0 > 0.0031308) ? (1.055 * powf(r0, 0.416666) - 0.055) : 12.92 * r0;
	g0 = (g0 > 0.0031308) ? (1.055 * powf(g0, 0.416666) - 0.055) : 12.92 * g0;
	b0 = (b0 > 0.0031308) ? (1.055 * powf(b0, 0.416666) - 0.055) : 12.92 * b0;

	r0 = CLAMP(r0, 0.0, 1.0);
	g0 = CLAMP(g0, 0.0, 1.0);
	b0 = CLAMP(b0, 0.0, 1.0);

	*R = (BYTE)(r0 * 255.f);
	*G = (BYTE)(g0 * 255.f);
	*B = (BYTE)(b0 * 255.f);
}

void color_rgb2ycbcr(BYTE R, BYTE G, BYTE B, BYTE * y, BYTE * cb, BYTE * cr)
{
	int y2, cb2, cr2;
	RGB_TO_YCBCR(R, G, B, y2, cb2, cr2);
	*y = (BYTE) y2;
	*cb = (BYTE) cb2;
	*cr = (BYTE) cr2;
}

// Y, U, V
// Y = 0.299R + 0.587G + 0.114B
// U = -0.147R - 0.289G + 0.436B
// V = 0.615R - 0.515G - 0.100B
void color_rgb2gray(BYTE r, BYTE g, BYTE b, BYTE * gray)
{
	//  float d = 0.299f * r + 0.587 * g + 0.114 * b;
	// *gray = (BYTE)(d + 0.5f);
	int d = (306 * r + 601 * g + 117 * b) >> 10;
	*gray = (BYTE) d;
}

void color_rgbsort(int n, RGBA_8888 * cv[])
{
	qsort(cv, n, sizeof(RGBA_8888 *), __color_rgbcmp);
}

int color_midval(IMAGE * img, char orgb)
{
	float midv;
	return (image_statistics(img, orgb, &midv, NULL) == RET_OK) ? (int) midv : -1;
}

VECTOR *color_vector(IMAGE * img, RECT * rect, int ndim)
{
	IMAGE *sub = NULL;
	int i, *count = NULL;
	VECTOR *vec = vector_create(ndim);
	CHECK_VECTOR(vec);

	if (rect) {
		sub = image_subimg(img, rect);
		CHECK_IMAGE(sub);
		count = color_count(sub, 1, 1, ndim);
		image_destroy(sub);
	} else {					// whole image
		count = color_count(img, 1, 1, ndim);
	}

	if (count) {
		for (i = 0; i < ndim; i++)
			vec->ve[i] = count[i];
		free(count);
	}
	vector_normal(vec);

	return vec;
}

float color_likeness(IMAGE * f, IMAGE * g, RECT * rect, int ndim)
{
	float avgd;
	VECTOR *fs, *gs;

	fs = color_vector(f, rect, ndim);
	check_vector(fs);
	gs = color_vector(g, rect, ndim);
	check_vector(gs);

	avgd = vector_likeness(fs, gs);

	vector_destroy(fs);
	vector_destroy(gs);

	return avgd;
}

int skin_detect(IMAGE * img)
{
	int i, j;
	check_image(img);
	image_foreach(img, i, j) {
		if (!color_beskin(img->ie[i][j].r, img->ie[i][j].g, img->ie[i][j].b)) {
			img->ie[i][j].r = 0;
			img->ie[i][j].g = 0;
			img->ie[i][j].b = 0;
		}
	}
	return RET_OK;
}

// r,g,b, w, class
MATRIX *color_classmat(IMAGE * image)
{
	int i, j, k, n, weight[0xffff + 1];
	int r, g, b;
	MATRIX *mat;

	memset(weight, 0, ARRAY_SIZE(weight) * sizeof(int));
	image_foreach(image, i, j) {
		k = RGB565_NO(image->ie[i][j].r, image->ie[i][j].g, image->ie[i][j].b);
		weight[k]++;
	}

	// Count no-zero colors
	for (n = 0, i = 0; i < ARRAY_SIZE(weight); i++) {
		if (weight[i])
			n++;
	}

	mat = matrix_create(n, 5);
	CHECK_MATRIX(mat);
	for (n = 0, i = 0; i < ARRAY_SIZE(weight); i++) {
		if (weight[i]) {
			r = RGB565_R(i);
			g = RGB565_G(i);
			b = RGB565_B(i);

			mat->me[n][0] = r;
			mat->me[n][1] = g;
			mat->me[n][2] = b;
			mat->me[n][3] = weight[i];
			n++;
		}
	}

	return mat;
}

int color_cluster(IMAGE * image, int num)
{
	int i, j, k, c;
	MATRIX *mat, *ccmat;
	int classno[0xffff + 1];

	check_image(image);

	mat = color_classmat(image);
	check_matrix(mat);

	// Weight k-means
	ccmat = matrix_wkmeans(mat, num, NULL);
	check_matrix(ccmat);

	matrix_print(ccmat, "%10.2f");

	// Back project !!!
	memset(classno, 0, ARRAY_SIZE(classno) * sizeof(int));
	for (i = 0; i < mat->m; i++) {
		k = RGB565_NO((int) mat->me[i][0], (int) mat->me[i][1], (int) mat->me[i][2]);
		classno[k] = (int) mat->me[i][4];	// weight -- class
	}
	image_foreach(image, i, j) {
		k = RGB565_NO(image->ie[i][j].r, image->ie[i][j].g, image->ie[i][j].b);
		c = classno[k];
		image->ie[i][j].a = (BYTE) c;	// Class index

		// if (update) {
		//         image->ie[i][j].r = (BYTE)ccmat->me[c][0];
		//         image->ie[i][j].g = (BYTE)ccmat->me[c][1];
		//         image->ie[i][j].b = (BYTE)ccmat->me[c][2];
		// }
		// image->ie[i][j].d = (BYTE)ccmat->me[c][5];      // Depths, Layers
	}

	image->format = IMAGE_MASK;
	image->K = num;
	for (c = 0; c < num && c < ARRAY_SIZE(image->KColors); c++) {
		image->KColors[c] = RGB_INT((BYTE) ccmat->me[c][0], (BYTE) ccmat->me[c][1], (BYTE) ccmat->me[c][2]);
		image->KCounts[c] = (int)ccmat->me[c][3]; // r, g, b, count, orig class, sorted class no
	}

	matrix_destroy(ccmat);
	matrix_destroy(mat);

	return RET_OK;
}

int *color_count(IMAGE * image, int rows, int cols, int levs)
{
	int *count;
	int i, j, i2, j2, k2, bh, bw;	// bh -- block height, bw -- block width

	if (levs < 1 || rows < 1 || cols < 1) {
		syslog_error("rows(%d), cols(%d), levs(%d) must be greater 1", rows, cols, levs);
		return NULL;
	}

	count = (int *) calloc((size_t) rows * cols * levs, sizeof(int));
	if (!count) {
		syslog_error("Allocate memeory.");
		return NULL;
	}

	color_cluster(image, levs);

	// Statistics !!!
	bh = (image->height + rows - 1) / rows;
	bw = (image->width + cols - 1) / cols;
	for (i = 0; i < image->height; i++) {
		i2 = (i / bh);
		for (j = 0; j < image->width; j++) {
			j2 = (j / bw);
			k2 = image->ie[i][j].a;
			count[CUBE_CELL_OFFSET(i2, j2, k2)]++;
		}
	}

	return count;
}

int color_picker()
{
	static int color_set[9] =
		{ 0x00ff00, 0x0000ff, 0x00ffff, 0xff00ff, 0xffff00, 0x7f0000, 0x007f00, 0x00ff7f, 0x007f7f };
	return color_set[random() % ARRAY_SIZE(color_set)];
}

int skin_statics(IMAGE * img, RECT * rect)
{
	int i, j, sum;
	check_image(img);

	sum = 0;
	for (i = rect->r; i < rect->r + rect->h; i++) {
		for (j = rect->c; j < rect->c + rect->w; j++) {
			if (color_beskin(img->ie[i][j].r, img->ie[i][j].g, img->ie[i][j].b))
				sum++;
		}
	}
	return sum;
}

// GWM -- Gray World Method
int color_rect_gwmgain(IMAGE * img, RECT * rect, float *r_gain, float *g_gain, float *b_gain)
{
	int i, j;
	float r_avg, g_avg, b_avg, rgb_avg, d;

	image_rectclamp(img, rect);

	if (rect->h < 1 || rect->w)
		return RET_ERROR;

	r_avg = g_avg = b_avg = 0.0f;
	rect_foreach(rect, i, j) {
		r_avg += img->ie[i + rect->r][j + rect->c].r;
		g_avg += img->ie[i + rect->r][j + rect->c].g;
		b_avg += img->ie[i + rect->r][j + rect->c].b;
	}

	d = 1.0f / rect->h / rect->w;
	r_avg *= d;
	g_avg *= d;
	b_avg *= d;

	rgb_avg = 127.0f;			// (r_avg + g_avg + b_avg)/3.0f;  // 127.0f

	*r_gain = *g_gain = *b_gain = 1.0f;
	if (r_avg > MIN_FLOAT_NUMBER)
		*r_gain = rgb_avg / r_avg;
	if (g_avg > MIN_FLOAT_NUMBER)
		*g_gain = rgb_avg / g_avg;
	if (b_avg > MIN_FLOAT_NUMBER)
		*b_gain = rgb_avg / b_avg;

	return RET_OK;
}

// GWM -- Gray World Method
int color_gwmgain(IMAGE * img, float *r_gain, float *g_gain, float *b_gain)
{
	RECT rect;

	image_rect(&rect, img);
	return color_rect_gwmgain(img, &rect, r_gain, g_gain, b_gain);
}

int color_balance(IMAGE * img, int method, int debug)
{
	float r_gain, g_gain, b_gain;

	if (method == COLOR_BALANCE_GRAY_WORLD) {
		color_gwmgain(img, &r_gain, &g_gain, &b_gain);
		if (debug)
			printf("GWM Gain: r = %lf, g = %lf, b = %lf\n", r_gain, g_gain, b_gain);
	} else {
		color_prmgain(img, &r_gain, &g_gain, &b_gain);
		if (debug)
			printf("PRM Gain: r = %lf, g = %lf, b = %lf\n", r_gain, g_gain, b_gain);
	}

	color_correct(img, r_gain, g_gain, b_gain);

	return RET_OK;
}

// PRM -- Perfect Reflector Method
int color_rect_prmgain(IMAGE * img, RECT * rect, float *r_gain, float *g_gain, float *b_gain)
{
	int i, j, k, threshold;
	float ratio, sum, r_max, g_max, b_max;
	int count[255 * 3 + 1];

	check_image(img);
	image_rectclamp(img, rect);

	memset(count, 0, ARRAY_SIZE(count) * sizeof(int));
	rect_foreach(rect, i, j) {
		count[img->ie[i + rect->r][j + rect->c].r + img->ie[i + rect->r][j + rect->c].g +
			  img->ie[i + rect->r][j + rect->c].b]++;
	}

	sum = 0.0f;
	ratio = 0.05f * img->height * img->width;
	threshold = 0;
	for (k = ARRAY_SIZE(count) - 1; k >= 0; k--) {
		sum += count[k];
		if (sum >= ratio) {
			threshold = k;
			break;
		}
	}

	// Count R+G+B >= threshold as white points
	r_max = g_max = b_max = sum = 0.0f;
	rect_foreach(rect, i, j) {
		if (img->ie[i + rect->r][j + rect->c].r + img->ie[i + rect->r][j + rect->c].g +
			img->ie[i + rect->r][j + rect->c].b >= threshold) {
			sum += 1.0f;
			r_max += img->ie[i + rect->r][j + rect->c].r;
			g_max += img->ie[i + rect->r][j + rect->c].g;
			b_max += img->ie[i + rect->r][j + rect->c].b;
		}
	}

	if (sum > 0.0f) {
		r_max = r_max / sum;
		g_max = g_max / sum;
		b_max = b_max / sum;
	}

	*r_gain = (r_max < 1) ? 1.0f : 255.0f / r_max;
	*g_gain = (g_max < 1) ? 1.0f : 255.0f / g_max;
	*b_gain = (b_max < 1) ? 1.0f : 255.0f / b_max;

	return RET_OK;
}

int color_prmgain(IMAGE * img, float *r_gain, float *g_gain, float *b_gain)
{
	RECT rect;

	image_rect(&rect, img);
	return color_rect_prmgain(img, &rect, r_gain, g_gain, b_gain);
}

// Von Kries diagonal model
int color_correct(IMAGE * img, float gain_r, float gain_g, float gain_b)
{
	int i, j;
	float d;

	check_image(img);

	image_foreach(img, i, j) {
		d = gain_r * img->ie[i][j].r;
		img->ie[i][j].r = (BYTE) MIN(d, 255);

		d = gain_g * img->ie[i][j].g;
		img->ie[i][j].g = (BYTE) MIN(d, 255);

		d = gain_b * img->ie[i][j].b;
		img->ie[i][j].b = (BYTE) MIN(d, 255);
	}

	return RET_OK;
}

int color_togray(IMAGE * img)
{
	BYTE n;
	int i, j;

	check_image(img);

	image_foreach(img, i, j) {
		color_rgb2gray(img->ie[i][j].r, img->ie[i][j].g, img->ie[i][j].b, &n);
		img->ie[i][j].r = img->ie[i][j].g = img->ie[i][j].b = n;
	}

	img->format = IMAGE_GRAY;

	return RET_OK;
}

int color_torgb565(IMAGE * img)
{
	int i, j;

	check_image(img);

	image_foreach(img, i, j) {
		img->ie[i][j].r = (img->ie[i][j].r >> 3) << 3;	// 5
		img->ie[i][j].g = (img->ie[i][j].g >> 2) << 2;	// 6
		img->ie[i][j].b = (img->ie[i][j].b >> 3) << 3;	// 5
	}
	img->format = IMAGE_RGB565;

	return RET_OK;
}

// Lab distance
float color_distance(RGBA_8888 * c1, RGBA_8888 * c2)
{
	static int first = 1;
	static float L[0xffff + 1], a[0xffff + 1], b[0xffff + 1];

	int i, j, k;
	int n1, n2;
	float dL, da, db;

	if (first) {
		for (i = 0; i <= 0xff; i++) {
			for (j = 0; j <= 0xff; j++) {
				for (k = 0; k <= 0xff; k++) {
					color_rgb2lab(i, j, k, &dL, &da, &db);
					n1 = RGB565_NO(i, j, k);
					L[n1] = dL;
					a[n1] = da;
					b[n1] = db;
				}
			}
		}
		first = 0;
	}

	n1 = RGB565_NO(c1->r, c1->g, c1->b);
	n2 = RGB565_NO(c2->r, c2->g, c2->b);
	dL = L[n1] - L[n2];
	dL *= dL;
	da = a[n1] - a[n2];
	da *= da;
	db = b[n1] - b[n2];
	db *= db;

	return sqrt(dL + da + db);
}
