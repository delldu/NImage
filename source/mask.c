
/************************************************************************************
***
***	Copyright 2020 Dell Du(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Sat Jul 31 14:19:59 HKT 2020
***
************************************************************************************/

#include "image.h"

#define MAX_STACK_ELEMENTS (4 * 1024 * 1024)

#define check_mask(img) \
	do { \
		if (! mask_valid(img)) { \
			syslog_error("Bad mask.\n"); \
			return RET_ERROR; \
		} \
	} while(0)

struct {
    int row[MAX_STACK_ELEMENTS], col[MAX_STACK_ELEMENTS], count;
} temp_stack;

inline void statck_reset()
{
    temp_stack.count = 0;
}

inline void stack_push(int row, int col)
{
    if (temp_stack.count < MAX_STACK_ELEMENTS) {
        temp_stack.row[temp_stack.count] = row;
        temp_stack.col[temp_stack.count] = col;
        temp_stack.count++;
    }
}

inline void stack_pop(int *row, int *col)
{
    *row = temp_stack.row[temp_stack.count - 1];
    *col = temp_stack.col[temp_stack.count - 1];
    temp_stack.count--;
}

inline int stack_empty()
{
    return temp_stack.count < 1;
}

#define start_tracking() \
do { \
	int i2, j2, row, col, start_row, start_col, stop_row, stop_col; \
    statck_reset(); \
    stack_push(i, j); \
    while(! stack_empty()) { \
        stack_pop(&row, &col); \
        image->ie[row][col].r = RGB_R(instance + 1); \
        image->ie[row][col].g = RGB_G(instance + 1); \
        image->ie[row][col].b = RGB_B(instance + 1); \
        start_row = MAX(0, row - KRadius); \
        stop_row = MIN(H - 1, row + KRadius); \
        start_col = MAX(0, col - KRadius); \
        stop_col = MIN(W - 1, col + KRadius); \
        for (i2 = start_row; i2 <= stop_row; i2++) { \
            for (j2 = start_col; j2 <= stop_col; j2++) { \
                if (image->ie[i2][j2].a == image->ie[row][col].a && \
                	RGB_INT(image->ie[i2][j2].r, image->ie[i2][j2].g, image->ie[i2][j2].b) == 0) \
                    stack_push(i2, j2); \
            } \
        } \
    } \
} while (0)

int mask_valid(MASK *img)
{
	if (! image_valid(img))
		return 0;
	return (img->format == IMAGE_MASK && img->K > 0)? 1 : 0;
}

int color_instance_(MASK *image, int KRadius)
{
    // KRadius -- radius, define neighbours
    int i, j, instance, H, W;

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
            instance++;
        }
    }
    image->KRadius = KRadius;
    image->KInstance = instance;

    return RET_OK;
}

int mask_show(MASK *mask)
{
	int i, j, color, label;
	float alpha;
	IMAGE *image;

	check_mask(mask);
	image = image_copy(mask); check_image(image);
	image_foreach(image, i, j) {
		label = RGB_INT(image->ie[i][j].r, image->ie[i][j].g, image->ie[i][j].b);
		color = image->KColors[image->ie[i][j].a];
		alpha = (label % 2 == 0)? 1.0 : 0.8;
		image->ie[i][j].r = (BYTE)(RGB_R(color) * alpha);
		image->ie[i][j].g = (BYTE)(RGB_G(color) * alpha);
		image->ie[i][j].b = (BYTE)(RGB_B(color) * alpha);
	}
	image_show(image, "mask");

	image_destroy(image);
	return RET_OK;
}




int mask_refine(MASK *semantic, MASK *color)
{
	MATRIX *mat;	// Insection matrix

	check_mask(semantic);
	check_mask(color);

	mat = matrix_create(semantic->K, color->K);
	check_matrix(mat);


	matrix_destroy(mat);

	return RET_OK;
}

// box select
int mask_drag(MASK *mask, RECT *rect) {
	check_mask(mask);
	return RET_OK;
}

// point select
int mask_click(MASK *mask, int row, int col) {
	check_mask(mask);
	return RET_OK;
}

