
/************************************************************************************
***
***	Copyright 2020 Dell Du(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Sat Jul 31 14:19:59 HKT 2020
***
************************************************************************************/

#include "image.h"
// 
int color_label(IMAGE *image, int n, int threshold) {
	check_image(image);

	color_cluster(image, n, 0);	// NO need update, because RGB will be used for label

	// image trace connection

	return RET_OK;
}


// int mask_finetune(IMAGE *semantic, MASK *color);

// IMAGE *color_blocks(int n, RGB *colors); ==> grid blocks , 64x64 per blocks


