/************************************************************************************
***
***	Copyright 2017-2020 Dell(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Mon Aug  7 10:24:38 CST 2017
***
************************************************************************************/

#include "image.h"

#define PHASH_ROWS 8
#define PHASH_COLS 8

#define check_MATRIX(mat)                                                      \
  do {                                                                         \
    if (!matrix_valid(mat)) {                                                  \
      syslog_error("Bad matrix.");                                             \
      return 0L;                                                               \
    }                                                                          \
  } while (0)

extern MATRIX *matrix_mean_filter(MATRIX * src, int r);

#if 0
int hash_hamming(HASH64 f1, HASH64 f2)
{
	HASH64 x = f1 ^ f2;
	const HASH64 m1 = 0x5555555555555555ULL;
	const HASH64 m2 = 0x3333333333333333ULL;
	const HASH64 h01 = 0x0101010101010101ULL;
	const HASH64 m4 = 0x0f0f0f0f0f0f0f0fULL;
	x -= (x >> 1) & m1;
	x = (x & m2) + ((x >> 2) & m2);
	x = (x + (x >> 4)) & m4;

	return (x * h01) >> 56;
}

#else
int hash_hamming(HASH64 f1, HASH64 f2)
{
	int h;
	HASH64 d;

	h = 0;
	d = f1 ^ f2;
	while (d) {
		h++;
		d &= d - 1;
	}

	return h;
}
#endif

float hash_likeness(HASH64 f1, HASH64 f2)
{
	float t;

	t = hash_hamming(f1, f2);
	t /= (8.0f * sizeof(HASH64));

	return 1.0f - t;
}

void hash_dump(char *title, HASH64 finger)
{
	int i, j, k;

	printf("%s Fingerprint: 0x%lx\n", title, finger);
	for (i = 0; i < PHASH_ROWS; i++) {
		for (j = 0; j < PHASH_COLS; j++) {
			k = PHASH_ROWS * PHASH_COLS - (i * PHASH_COLS + j) - 1;
			printf("%d ", (int) ((finger >> k) & 0x1));
		}
		printf("\n");
	}
}

// Perception hash
HASH64 image_phash(IMAGE * image, char oargb, RECT * rect)
{
	static int dct_init = 0;
	static MATRIX *dct1 = NULL;
	static MATRIX *dct2 = NULL;

	int i, j, k;
	float a, c, m;
	MATRIX *mat, *mat32, *smat;	// smat -- smooth mat
	HASH64 finger = 0L;

	// Init DCT32 matrix
	if (!dct_init) {
		dct1 = matrix_create(32, 32);
		check_MATRIX(dct1);
		c = MATH_PI / 2 / 32;
		for (i = 0; i < dct1->m; i++) {
			a = (i == 0) ? sqrt(1.0 / 32) : sqrt(2.0 / 32);
			for (j = 0; j < dct1->n; j++) {
				dct1->me[i][j] = a * cos(c * i * (2 * j + 1));
			}
		}
		dct2 = matrix_transpose(dct1);
		check_MATRIX(dct2);
		dct_init = 1;
	}
	if (rect)
		mat = image_rect_plane(image, oargb, rect);
	else
		mat = image_getplane(image, oargb);
	check_MATRIX(mat);

	smat = matrix_mean_filter(mat, 3);
	check_MATRIX(smat);

	matrix_destroy(mat);
	mat32 = matrix_zoom(smat, 32, 32, ZOOM_METHOD_BLINE);
	check_MATRIX(mat32);
	matrix_destroy(smat);

	mat = matrix_create(32, 32);
	check_MATRIX(mat);
	// mat = mat32 * dct2
	matrix_multi(mat, mat32, dct2);
	// mat32 = dct1 * mat32
	matrix_multi(mat32, dct1, mat);
	matrix_destroy(mat);

	mat = matrix_create(PHASH_ROWS, PHASH_COLS);
	check_MATRIX(mat);
	// Save DCT Result Data to mat !
	matrix_foreach(mat, i, j) mat->me[i][j] = mat32->me[i + 1][j + 1];
	matrix_destroy(mat32);

	m = matrix_median(mat);

	finger = 0L;
	for (i = 0; i < PHASH_ROWS; i++) {
		for (j = 0; j < PHASH_COLS; j++) {
			k = PHASH_ROWS * PHASH_COLS - (i * PHASH_COLS + j) - 1;
			if (mat->me[i][j] > m)
				finger |= (0x01L << k);
		}
	}
	matrix_destroy(mat);

	return finger;
}

// average hash
HASH64 image_ahash(IMAGE * image, char oargb, RECT * rect)
{
	BYTE n;
	HASH64 finger;
	int i, j, k, i2, j2, bh, bw, avg, count[PHASH_ROWS][PHASH_COLS];
	// bh -- block height, bw -- block width

	image_rectclamp(image, rect);

	bh = rect->h / PHASH_ROWS;
	bw = rect->w / PHASH_COLS;

	rect->h = bh * PHASH_ROWS;
	rect->w = bw * PHASH_COLS;

	// Statistics
	k = avg = 0;
	memset(count, 0, PHASH_ROWS * PHASH_COLS * sizeof(int));

	switch (oargb) {
	case 'A':
		rect_foreach(rect, i, j) {
			i2 = (int) (i / bh);
			j2 = (int) (j / bw);
			color_rgb2gray(image->ie[i + rect->r][j + rect->c].r,
						   image->ie[i + rect->r][j + rect->c].g, image->ie[i + rect->r][j + rect->c].b, &n);
			n /= 4;				// 64 Level Gray
			count[i2][j2] += n;
			avg += n;
			k++;
		}
		break;
	case 'R':
		rect_foreach(rect, i, j) {
			i2 = (int) (i / bh);
			j2 = (int) (j / bw);
			n = image->ie[i + rect->r][j + rect->c].r;
			n /= 4;				// 64 Level Gray
			count[i2][j2] += n;
			avg += n;
			k++;
		}
		break;
	case 'G':
		rect_foreach(rect, i, j) {
			i2 = (int) (i / bh);
			j2 = (int) (j / bw);
			n = image->ie[i + rect->r][j + rect->c].g;
			n /= 4;				// 64 Level Gray
			count[i2][j2] += n;
			avg += n;
			k++;
		}
		break;

	case 'B':
		rect_foreach(rect, i, j) {
			i2 = (int) (i / bh);
			j2 = (int) (j / bw);
			n = image->ie[i + rect->r][j + rect->c].b;
			n /= 4;				// 64 Level Gray
			count[i2][j2] += n;
			avg += n;
			k++;
		}

		break;
	default:
		return 0L;
		break;
	}

	// Average
	avg /= k;
	for (i = 0; i < PHASH_ROWS; i++) {
		for (j = 0; j < PHASH_COLS; j++) {
			count[i][j] /= (bh * bw);
			count[i][j] = (count[i][j] > avg) ? 1 : 0;
		}
	}

	// Collect fingerprint
	finger = 0L;
	for (i = 0; i < PHASH_ROWS; i++) {
		for (j = 0; j < PHASH_COLS; j++) {
			k = PHASH_ROWS * PHASH_COLS - (i * PHASH_COLS + j) - 1;
			if (count[i][j])
				finger |= (0x01L << k);
		}
	}

	return finger;
}

HASH64 shape_hash(IMAGE * image, RECT * rect)
{
	int i, j, k;
	float avg;
	VECTOR *vector;
	HASH64 finger;

	check_image(image);

	vector = shape_vector(image, rect, 60);
	check_vector(vector);

	avg = vector_mean(vector);

	// Collect fingerprint
	finger = 0L;
	for (i = 0; i < PHASH_ROWS; i++) {
		for (j = 0; j < PHASH_COLS; j++) {
			k = PHASH_ROWS * PHASH_COLS - (i * PHASH_COLS + j) - 1;
			if (k < 60 && vector->ve[k] > avg)
				finger |= (0x01L << k);
		}
	}

	vector_destroy(vector);

	return finger;
}

HASH64 texture_hash(IMAGE * image, RECT * rect)
{
	int i, j, k;
	float avg;
	VECTOR *vector;
	HASH64 finger;

	if (!image_valid(image))
		return 0L;

	vector = texture_vector(image, rect, PHASH_ROWS * PHASH_COLS);
	if (!vector_valid(vector))
		return 0L;

	avg = vector_mean(vector);

	// Collect fingerprint
	finger = 0L;
	for (i = 0; i < PHASH_ROWS; i++) {
		for (j = 0; j < PHASH_COLS; j++) {
			k = PHASH_ROWS * PHASH_COLS - (i * PHASH_COLS + j) - 1;
			if (vector->ve[k] > avg)
				finger |= (0x01L << k);
		}
	}
	vector_destroy(vector);

	return finger;
}
