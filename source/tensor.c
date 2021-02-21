/************************************************************************************
***
***	Copyright 2021 Dell(18588220928g@163.com), All Rights Reserved.
***
***	File Author: Dell, 2021-01-09 14:43:18
***
************************************************************************************/

#include "tensor.h"
#include "image.h"
#include "matrix.h"

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

TENSOR *tensor_from_image(IMAGE * image, int with_alpha)
{
	int i, j;
	TENSOR *tensor;
	float *R, *G, *B, *A;

	CHECK_IMAGE(image);

	if (with_alpha)
		tensor = tensor_create(1, sizeof(RGBA_8888), image->height, image->width);
	else
		tensor = tensor_create(1, 3, image->height, image->width);	// RGB
	CHECK_TENSOR(tensor);

	R = tensor->data;
	G = R + tensor->height * tensor->width;
	B = G + tensor->height * tensor->width;
	if (with_alpha)
		A = B + tensor->height * tensor->width;

	image_foreach(image, i, j) {
		*R++ = (float) image->ie[i][j].r / 255.0;
		*G++ = (float) image->ie[i][j].g / 255.0;
		*B++ = (float) image->ie[i][j].b / 255.0;
	}
	if (with_alpha) {
		image_foreach(image, i, j)
			*A++ = (float) image->ie[i][j].a / 255.0;
	}

	return tensor;
}

float *tensor_start_row(TENSOR *tensor, int b, int c, int h)
{
	int offset;
	if (b < 0 || b >= tensor->batch || c < 0 || c >= tensor->chan || h < 0 || h >= tensor->height)
		return NULL;
	offset = b * tensor->chan * tensor->height * tensor->width \
			+ c * tensor->height * tensor->width \
			+ h * tensor->width;
	return tensor->data + offset;
}

float *tensor_start_chan(TENSOR *tensor, int b, int c)
{
	int offset;
	if (b < 0 || b >= tensor->batch || c < 0 || c >= tensor->chan)
		return NULL;
	offset = b * tensor->chan * tensor->height * tensor->width \
			+ c * tensor->height * tensor->width;
	return tensor->data + offset;
}

TENSOR *tensor_zoom(TENSOR *source, int nh, int nw)
{
	int b, c;
	MATRIX *s_mat, *d_mat;
	float *s_data, *d_data;
	TENSOR *zoom = NULL;

	CHECK_TENSOR(source);
	zoom = tensor_create(source->batch, source->chan, nh, nw); CHECK_TENSOR(zoom);

	s_mat = matrix_create(source->height, source->width); CHECK_MATRIX(s_mat);
	for (b = 0; b < source->batch; b++) {
		for (c = 0; c < source->chan; c++) {
			s_data = tensor_start_chan(source, b, c);
			memcpy(s_mat->base, s_data, s_mat->m * s_mat->n * sizeof(float));
			d_mat = matrix_zoom(s_mat, nh, nw, ZOOM_METHOD_BLINE);
			if (matrix_valid(d_mat)) {
				d_data = tensor_start_chan(zoom, b, c);
				memcpy(d_data, d_mat->base, nh * nw * sizeof(float));
				matrix_destroy(d_mat);
			}
		}
	}
	matrix_destroy(s_mat);

	return zoom;
}

TENSOR *tensor_rgb2lab(IMAGE *image)
{
	int i, j;
	TENSOR *tensor;
	float *R, *G, *B, *A, L, a, b;

	CHECK_IMAGE(image);

	tensor = tensor_create(1, sizeof(RGBA_8888), image->height, image->width);
	CHECK_TENSOR(tensor);

	R = tensor->data;
	G = R + tensor->height * tensor->width;
	B = G + tensor->height * tensor->width;
	A = B + tensor->height * tensor->width;

	image_foreach(image, i, j) {
		color_rgb2lab(image->ie[i][j].r, image->ie[i][j].g, image->ie[i][j].b, &L, &a, &b);
		L = (L - 50.f)/100.f; a /= 110.f; b /= 110.f;
		*R++ = L; *G++ = a; *B++ = b; *A++ = (image->ie[i][j].a > 127)? 1.0 : 0.f;
	}

	return tensor;
}

IMAGE *tensor_lab2rgb(TENSOR *tensor, int k)
{
	int i, j;
	IMAGE *image;
	BYTE r0, g0, b0;
	float *R, *G, *B, *A, L, a, b;

	CHECK_TENSOR(tensor);

	if (k < 0 || k >= tensor->batch) {
		syslog_error("image index over tensor batch size.");
		return NULL;
	}
	if (tensor->chan < 3) {
		syslog_error("tensor channel < 3, it's difficult to create image.");
		return NULL;
	}

	image = image_create(tensor->height, tensor->width); CHECK_IMAGE(image);
	R = tensor->data + k * (tensor->chan * tensor->height * tensor->width);
	G = R + tensor->height * tensor->width;
	B = G + tensor->height * tensor->width;
	A = B + tensor->height * tensor->width;

	image_foreach(image, i, j) {
		L = *R++; a = *G++; b = *B++;
		L = L * 100.f + 50.f; a *= 110.f; b *= 110.f;
		color_lab2rgb(L, a, b, &r0, &g0, &b0);
		image->ie[i][j].r = r0;
		image->ie[i][j].g = g0;
		image->ie[i][j].b = b0;
		image->ie[i][j].a = 255;
	}

	if (tensor->chan >= 4) {
		image_foreach(image, i, j)
			image->ie[i][j].a = (BYTE) ((*A++) * 255);
	}

	return image;
}

int tensor_setmask(TENSOR *tensor, float mask)
{
	int i, j;
	float *alpha;

	check_tensor(tensor);
	if (tensor->chan < 4)
		return RET_ERROR;

	alpha = tensor_start_chan(tensor, 0 /*batch*/, 3 /*channel*/);
	for (i = 0; i < tensor->height; i++) {
		for (j = 0; j < tensor->width; j++)
			*alpha++ = mask;
	}
	return RET_OK;
}