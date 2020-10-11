/************************************************************************************
***
***	Copyright 2017-2020 Dell(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Wed Jun 14 18:05:59 PDT 2017
***
************************************************************************************/


#include "image.h"
#include "filter.h"

#define MASK_THRESHOLD 128
#define COLOR_CLUSTERS 32
#define BOARDER_THRESHOLD 5
#define MASK_ALPHA 0.0f

#define MAX_MULTI_BAND_LAYERS 16


#define mask_foreach(img, i, j) \
	for (i = 1; i < img->height - 1; i++) \
		for (j = 1; j < img->width - 1; j++)


static void __mask_binary(IMAGE *mask, int debug)
{
	int i, j;

	image_foreach(mask, i, j) {
		// set 0 to box border
		if (i == 0 || j == 0 || i == mask->height - 1 || j == mask->width - 1)
			mask->ie[i][j].g = 0;
		else	
			mask->ie[i][j].g = (mask->ie[i][j].g < MASK_THRESHOLD)? 0 : 255;
	}

	if (debug) {
		image_foreach(mask, i, j) {
			mask->ie[i][j].r = mask->ie[i][j].g;
			mask->ie[i][j].b = mask->ie[i][j].g;
		}
	}
}

static inline int __mask_border(IMAGE *mask, int i, int j)
{
	int k;
	static int nb[4][2] = { {0, 1}, {-1, 0}, {0, -1}, {1, 0}};

	if (mask->ie[i][j].g != 255)
		return 0;

	// current is 1
	for (k = 0; k < 4; k++) {
		if (mask->ie[i + nb[k][0]][j + nb[k][1]].g == 0)
			return 1;
	}

	return 0;
}

// neighbours
static inline int __mask_4conn(IMAGE *mask, int i, int j)
{
	int k, sum;
	static int nb[4][2] = { {0, 1}, {-1, 0}, {0, -1}, {1, 0}};

	sum = 0;
	for (k = 0; k < 4; k++) {
		if (mask->ie[i + nb[k][0]][j + nb[k][1]].g != 0)
			sum++;
	}

	return sum;
}


static inline int __mask_cloner(IMAGE *mask, int i, int j)
{
  	return (mask->ie[i][j].g == 255);
}


static inline void __mask_update(IMAGE *mask, int i, int j)
{
	mask->ie[i][j].r = 255;
	mask->ie[i][j].g = 255;
	mask->ie[i][j].b = 255;
}

static void __mask_remove(IMAGE *mask, int i, int j)
{
	mask->ie[i][j].r = 0;
	mask->ie[i][j].g = 0;
	mask->ie[i][j].b = 0;
}

static void __mask_finetune(IMAGE *mask, IMAGE *src, int debug)
{
	int i, j, k, count[COLOR_CLUSTERS];

	__mask_binary(mask, debug);

	color_cluster(src, COLOR_CLUSTERS, 0);	// not update

	// Calculate border colors
	memset(count, 0, COLOR_CLUSTERS * sizeof(int));
	mask_foreach(mask, i, j) {
		if (! __mask_border(mask, i, j))
			continue;
		count[src->ie[i][j].a]++;
	}

	// Fast delete border's blocks
	for (k = 0; k < COLOR_CLUSTERS; k++) {
		if (count[k] < BOARDER_THRESHOLD)
			continue;
		mask_foreach(src, i, j) {
			if (src->ie[i][j].a == k) {
				__mask_remove(mask, i, j);
			}
		}
	}

	// Delete isolate points
	mask_foreach(mask,i,j) {
		if (! __mask_cloner(mask, i, j))
			continue;

		// current == 255
		k = __mask_4conn(mask, i, j);
		if (k <= 1)
			__mask_remove(mask, i, j);
	}

	// Fill isolate holes
	mask_foreach(mask,i,j) {
		if (__mask_cloner(mask, i, j))
			continue;

		// ==> current == 0
		k = __mask_4conn(mask, i, j);
		if (k >= 3)
			__mask_update(mask, i, j);
	}
}


int blend_cluster(IMAGE *src, IMAGE *mask, IMAGE *dst, int top, int left, int debug)
{
	int i, j;
	double d;

	check_image(src);
	check_image(dst);

	if (debug) {
		time_reset();
	}

	color_cluster(src, COLOR_CLUSTERS, 1);
	if (mask == NULL) {
		image_foreach(mask,i,j) {
			if (i == 0 || j == 0 || i == mask->height - 1 || j == mask->width - 1)
				mask->ie[i][j].r = mask->ie[i][j].g = mask->ie[i][j].b = 0;
			else
				mask->ie[i][j].r = mask->ie[i][j].g = mask->ie[i][j].b = 255;
		}
	}

	__mask_finetune(mask, src, debug);

	if (debug) {
		image_foreach(mask, i, j) {
			if (__mask_cloner(mask, i, j))
				continue;
			src->ie[i][j].r = 0;
			src->ie[i][j].g = 0;
			src->ie[i][j].b = 0;
		}
	}

	image_foreach(mask, i, j) {
		if (! __mask_cloner(mask, i, j))
			continue;

		if (image_outdoor(dst, i, top, j, left)) {
			// syslog_error("Out of target image");
			continue;
		}

		if (__mask_border(mask, i, j)) {
			d = MASK_ALPHA * src->ie[i][j].r + (1 - MASK_ALPHA)*dst->ie[i + top][j + left].r;
 			dst->ie[i + top][j + left].r = (BYTE)d;

			d = MASK_ALPHA * src->ie[i][j].g + (1 - MASK_ALPHA)*dst->ie[i + top][j + left].g;
 			dst->ie[i + top][j + left].g = (BYTE)d;

			d = MASK_ALPHA * src->ie[i][j].b + (1 - MASK_ALPHA)*dst->ie[i + top][j + left].b;
 			dst->ie[i + top][j + left].b = (BYTE)d;
		}
		else {
			dst->ie[i + top][j + left].r = src->ie[i][j].r;
			dst->ie[i + top][j + left].g = src->ie[i][j].g;
			dst->ie[i + top][j + left].b = src->ie[i][j].b;
		}
	}

	if (debug) {
		time_spend("Color cluster blending");
	}

	return RET_OK;
}
