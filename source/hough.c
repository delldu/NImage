
/************************************************************************************
***
***	Copyright 2010-2020 Dell Du(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Sat Jul 31 14:19:59 HKT 2010
***
************************************************************************************/

// Hough transform:
//   Line: r = y * cos(theta) + x * sin(theta) <==> (r, theta)
//   Circle:  x = x0 + r * cos(theta), y = y0 + r * sin(theta)
//   Oval:  x = x0 + a * cos(theta), y = y0 + b * sin(theta)

#include "vector.h"
#include "matrix.h"
#include "image.h"
#include <math.h>

#define MAX_LINE_NO 2048

typedef struct {
	int r1, c1, r2, c2;
} LINE;
typedef struct {
	int count;
	LINE line[MAX_LINE_NO];
} LINES;

static LINES __global_line_set;

LINES *line_set()
{
	return &__global_line_set;
}

void line_put(int r1, int c1, int r2, int c2)
{
	LINES *lines = line_set();

	if (lines->count < MAX_LINE_NO) {
		lines->line[lines->count].r1 = r1;
		lines->line[lines->count].c1 = c1;
		lines->line[lines->count].r2 = r2;
		lines->line[lines->count].c2 = c2;
		lines->count++;
	}
}

static int __hough_line(IMAGE * image, int threshold, int debug)
{
#define MAX_ANGLES 180
	int i, j, n, r, rmax;
	float d;
	VECTOR *cosv, *sinv;
	MATRIX *hough, *begrow, *endrow, *begcol, *endcol;
	LINES *lines = line_set();

	lines->count = 0;

	check_image(image);
	cosv = vector_create(180);
	check_vector(cosv);
	sinv = vector_create(180);
	check_vector(sinv);
	for (i = 0; i < 180; i++) {
		sinv->ve[i] = sin(i * MATH_PI / 180);
		cosv->ve[i] = cos(i * MATH_PI / 180);
	}

	rmax = (int) sqrt(image->height * image->height + image->width * image->width) + 1;
	hough = matrix_create(2 * rmax, MAX_ANGLES);
	check_matrix(hough);
	begrow = matrix_create(2 * rmax, MAX_ANGLES);
	check_matrix(begrow);
	endrow = matrix_create(2 * rmax, MAX_ANGLES);
	check_matrix(endrow);
	begcol = matrix_create(2 * rmax, MAX_ANGLES);
	check_matrix(begcol);
	endcol = matrix_create(2 * rmax, MAX_ANGLES);
	check_matrix(endcol);
//  shape_midedge(image);
	shape_bestedge(image);

	image_foreach(image, i, j) {
		if (image->ie[i][j].r < 128)
			continue;
		for (n = 0; n < MAX_ANGLES; n++) {
			d = i * cosv->ve[n] + j * sinv->ve[n];
			r = (int) (d + rmax);
			hough->me[r][n]++;
			if ((int) hough->me[r][n] == 1) {
				begrow->me[r][n] = i;
				begcol->me[r][n] = j;
			}
			endrow->me[r][n] = i;
			endcol->me[r][n] = j;
		}
	}

	matrix_foreach(hough, i, j) {
		if ((int) hough->me[i][j] >= threshold) {
			line_put((int) begrow->me[i][j], (int) begcol->me[i][j], (int) endrow->me[i][j], (int) endcol->me[i][j]);
			if (debug)
				image_drawline(image, (int) begrow->me[i][j], (int) begcol->me[i][j], (int) endrow->me[i][j],
							   (int) endcol->me[i][j], 0x00ffff);
		}
	}

	matrix_destroy(hough);
	matrix_destroy(begrow);
	matrix_destroy(endrow);
	matrix_destroy(begcol);
	matrix_destroy(endcol);

	return RET_OK;
}

int line_detect(IMAGE * img, int debug)
{
	return __hough_line(img, MIN(img->height, img->width) / 10, debug);
}

int line_lsm(IMAGE * image, RECT * rect, float *k, float *b, int debug)
{
	int i, j, n, x[4096], y[4096];
	float avg_xy, avg_x, avg_y, avg_xx, t;

	check_image(image);

	image_rectclamp(image, rect);
	n = 0;
	rect_foreach(rect, i, j) {
		if (image->ie[rect->r + i][rect->c + j].r < 128)
			continue;
		x[n] = rect->c + j;
		y[n] = rect->r + i;
		n++;

		if (n >= ARRAY_SIZE(x))
			break;
	}
	if (n < 2) {
		return RET_ERROR;
	}

	avg_xy = avg_x = avg_y = avg_xx = 0.0f;
	for (i = 0; i < n; i++) {
		avg_xy += x[i] * y[i];
		avg_x += x[i];
		avg_y += y[i];
		avg_xx += x[i] * x[i];
	}
	avg_xy /= n;
	avg_x /= n;
	avg_y /= n;
	avg_xx /= n;
	t = avg_xx - (avg_x) * (avg_x);

	if (t < MIN_FLOAT_NUMBER)
		return RET_ERROR;

	*k = (avg_xy - avg_x * avg_y) / t;
	*b = avg_y - (*k) * avg_x;

	if (debug) {
		image_drawkxb(image, *k, *b, color_picker());
	}

	return RET_OK;
}
