
/************************************************************************************
***
***	Copyright 2020 Dell Du(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Sat Jul 31 14:19:59 HKT 2020
***
************************************************************************************/

#include "image.h"

#define MIN_STACK_ELEMENTS 1024
#define MAX_STACK_ELEMENTS (4 * 1024 * 1024)
#define NOISE_LABEL 0xffffff

#define check_mask(img) \
	do { \
		if (! mask_valid(img)) { \
			syslog_error("Bad mask.\n"); \
			return RET_ERROR; \
		} \
	} while(0)

struct {
	int row[MAX_STACK_ELEMENTS], col[MAX_STACK_ELEMENTS], count;
} max_stack;

struct {
	int row[MIN_STACK_ELEMENTS], col[MIN_STACK_ELEMENTS], count;
} min_stack;

inline void max_statck_reset()
{
	max_stack.count = 0;
}

inline void max_stack_push(int row, int col)
{
	if (max_stack.count < MAX_STACK_ELEMENTS) {
		max_stack.row[max_stack.count] = row;
		max_stack.col[max_stack.count] = col;
		max_stack.count++;
	}
}

inline void max_stack_pop(int *row, int *col)
{
	*row = max_stack.row[max_stack.count - 1];
	*col = max_stack.col[max_stack.count - 1];
	max_stack.count--;
}

inline int max_stack_empty()
{
	return max_stack.count < 1;
}

inline void min_statck_reset()
{
	min_stack.count = 0;
}

inline void min_stack_push(int row, int col)
{
	if (min_stack.count < MIN_STACK_ELEMENTS) {
		min_stack.row[min_stack.count] = row;
		min_stack.col[min_stack.count] = col;
		min_stack.count++;
	}
}

inline void min_stack_pop(int *row, int *col)
{
	*row = min_stack.row[min_stack.count - 1];
	*col = min_stack.col[min_stack.count - 1];
	min_stack.count--;
}

inline int min_stack_empty()
{
	return min_stack.count < 1;
}

#define start_tracking() \
do { \
	int i2, j2, row, col, start_row, start_col, stop_row, stop_col; \
    max_statck_reset(); \
    min_statck_reset(); \
    max_stack_push(i, j); \
    while(! max_stack_empty()) { \
        max_stack_pop(&row, &col); \
        image->ie[row][col].r = RGB_R(instance + 1); \
        image->ie[row][col].g = RGB_G(instance + 1); \
        image->ie[row][col].b = RGB_B(instance + 1); \
        min_stack_push(row, col); \
        start_row = MAX(0, row - KRadius); \
        stop_row = MIN(H - 1, row + KRadius); \
        start_col = MAX(0, col - KRadius); \
        stop_col = MIN(W - 1, col + KRadius); \
        for (i2 = start_row; i2 <= stop_row; i2++) { \
            for (j2 = start_col; j2 <= stop_col; j2++) { \
                if (image->ie[i2][j2].a == image->ie[row][col].a && \
                	RGB_INT(image->ie[i2][j2].r, image->ie[i2][j2].g, image->ie[i2][j2].b) == 0) \
                    max_stack_push(i2, j2); \
            } \
        } \
    } \
} while (0)

int mask_valid(MASK * img)
{
	if (!image_valid(img))
		return 0;
	return (img->format == IMAGE_MASK && img->K > 0) ? 1 : 0;
}

int color_instance_(MASK * image, int KRadius)
{
	// KRadius -- radius, define neighbours
	int i, j, instance, H, W, noise_row, noise_col;

	check_mask(image);

	image_foreach(image, i, j)
		image->ie[i][j].r = image->ie[i][j].g = image->ie[i][j].b = 0;	// Clear all labels

	instance = 0;
	H = image->height;
	W = image->width;
	for (i = 0; i < image->height; i++) {
		for (j = 0; j < image->width; j++) {
			if (RGB_INT(image->ie[i][j].r, image->ie[i][j].g, image->ie[i][j].b) > 0)
				continue;
			// New tracking
			start_tracking();
			if (min_stack.count < (2 * KRadius + 1) * (2 * KRadius + 1)) {
				while (!min_stack_empty()) {
					min_stack_pop(&noise_row, &noise_col);
					image->ie[noise_row][noise_col].r = RGB_R(NOISE_LABEL);
					image->ie[noise_row][noise_col].g = RGB_G(NOISE_LABEL);
					image->ie[noise_row][noise_col].g = RGB_B(NOISE_LABEL);
				}
			} else {
				instance++;
			}
		}
	}
	image->KRadius = KRadius;
	image->KInstance = instance;
	// CheckPoint("image->KInstance: %d", instance);

	return RET_OK;
}

static inline int __label_border(IMAGE * mask, int i, int j)
{
	int k, label1, label2;
	static int nb[4][2] = { {0, 1}, {1, 0}, {-1, 0}, {0, -1} };

	if (i == 0 || i == mask->height - 1 || j == 0 || j == mask->width - 1)
		return 0;

	label1 = RGB_INT(mask->ie[i][j].r, mask->ie[i][j].g, mask->ie[i][j].b);
	if (label1 == NOISE_LABEL)
		return 0;
	// current is 1
	for (k = 0; k < 2; k++) {
		label2 = RGB_INT(mask->ie[i + nb[k][0]][j + nb[k][1]].r,
						 mask->ie[i + nb[k][0]][j + nb[k][1]].g, mask->ie[i + nb[k][0]][j + nb[k][1]].b);
		if (label2 == NOISE_LABEL)
			return 0;
		if (label1 != label2)
			return 1;
	}
	return 0;
}

int mask_show(MASK * mask)
{
	int i, j, color;
	IMAGE *image;

	check_mask(mask);
	image = image_copy(mask);
	check_image(image);
	image_foreach(image, i, j) {
		// label = RGB_INT(image->ie[i][j].r, image->ie[i][j].g, image->ie[i][j].b);
		if (__label_border(mask, i, j)) {
			color = 0x7f0000;
		} else {
			color = image->KColors[image->ie[i][j].a];
		}
		image->ie[i][j].r = (BYTE) (RGB_R(color));
		image->ie[i][j].g = (BYTE) (RGB_G(color));
		image->ie[i][j].b = (BYTE) (RGB_B(color));
	}
	image_show(image, "mask");

	image_destroy(image);
	return RET_OK;
}

int mask_refine(MASK * semantic, MASK * color)
{
	MATRIX *mat;				// Insection matrix

	check_mask(semantic);
	check_mask(color);

	mat = matrix_create(semantic->K, color->K);
	check_matrix(mat);


	matrix_destroy(mat);

	return RET_OK;
}


#if 0
// box select
int mask_drag(MASK * mask, RECT * rect)
{
	check_mask(mask);
	return RET_OK;
}

// point select
int mask_click(MASK * mask, int row, int col)
{
	check_mask(mask);
	return RET_OK;
}
#endif
