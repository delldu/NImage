/************************************************************************************
***
***	Copyright 2021 Dell(18588220928g@163.com), All Rights Reserved.
***
***	File Author: Dell, 2021-01-09 14:43:18
***
************************************************************************************/

#include "tensor.h"
#include "image.h"

#define TENSOR_MAGIC MAKE_FOURCC('T', 'E', 'N', 'S')

int tensor_valid(TENSOR * tensor)
{
	return (!tensor || tensor->batch < 1 || tensor->chan < 1 ||
			tensor->height < 1 || tensor->width < 1 || tensor->magic != TENSOR_MAGIC) ? 0 : 1;
}

TENSOR *tensor_create(WORD b, WORD c, WORD h, WORD w)
{
	TENSOR *t;

	BYTE *base = (BYTE *) calloc((size_t) 1, sizeof(TENSOR) + b * c * h * w * sizeof(float));
	if (!base) {
		syslog_error("Allocate memeory.");
		return NULL;
	}
	t = (TENSOR *) base;

	t->magic = TENSOR_MAGIC;
	t->batch = b;
	t->chan = c;
	t->height = h;
	t->width = w;
	t->data = (float *) (base + sizeof(TENSOR));

	return t;
}

TENSOR *tensor_copy(TENSOR * src)
{
	TENSOR *dst;

	CHECK_TENSOR(src);

	dst = tensor_create(src->batch, src->chan, src->height, src->width);
	if (tensor_valid(dst)) {
		memcpy(dst->data, src->data, src->batch * src->chan * src->height * src->width * sizeof(float));
	}

	return dst;
}

void tensor_dump(TENSOR * tensor)
{
	printf("Tensor dims: %dx%dx%dx%d\n", tensor->batch, tensor->chan, tensor->height, tensor->width);
}

void tensor_destroy(TENSOR * tensor)
{
	if (!tensor_valid(tensor))
		return;

	free(tensor);
}

IMAGE *image_from_tensor(TENSOR * tensor, int k)
{
	int i, j;
	IMAGE *image;
	float *R, *G, *B, *A;

	CHECK_TENSOR(tensor);

	if (k < 0 || k >= tensor->batch) {
		syslog_error("image index over tensor batch size.");
		return NULL;
	}

	image = image_create(tensor->height, tensor->width);
	CHECK_IMAGE(image);
	R = tensor->data + k * (tensor->chan * tensor->height * tensor->width);
	G = R + tensor->height * tensor->width;
	B = G + tensor->height * tensor->width;
	A = B + tensor->height * tensor->width;

	image_foreach(image, i, j) {
		image->ie[i][j].r = (BYTE) ((*R++) * 255);
	}

	if (tensor->chan >= 2) {
		image_foreach(image, i, j)
			image->ie[i][j].g = (BYTE) ((*G++) * 255);
	}

	if (tensor->chan >= 3) {
		image_foreach(image, i, j)
			image->ie[i][j].b = (BYTE) ((*B++) * 255);
	}

	if (tensor->chan >= 4) {
		image_foreach(image, i, j)
			image->ie[i][j].a = (BYTE) ((*A++) * 255);
	}

	return image;
}

TENSOR *tensor_from_image(IMAGE * image, int alpha)
{
	int i, j;
	TENSOR *tensor;
	float *R, *G, *B, *A;

	CHECK_IMAGE(image);

	tensor = tensor_create(1, sizeof(RGBA_8888), image->height, image->width);
	CHECK_TENSOR(tensor);

	R = tensor->data;
	G = R + tensor->height * tensor->width;
	B = G + tensor->height * tensor->width;
	image_foreach(image, i, j) {
		*R++ = (float) image->ie[i][j].r / 255.0;
		*G++ = (float) image->ie[i][j].g / 255.0;
		*B++ = (float) image->ie[i][j].b / 255.0;
	}
	if (alpha) {
		A = B + tensor->height * tensor->width;
		image_foreach(image, i, j)
			*A++ = (float) image->ie[i][j].a / 255.0;
	}

	return tensor;
}
