/************************************************************************************
***
***	Copyright 2021 Dell(18588220928g@163.com), All Rights Reserved.
***
***	File Author: Dell, 2021-01-09 14:44:13
***
************************************************************************************/


#ifndef _TENSOR_H
#define _TENSOR_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "common.h"

#define check_tensor(tensor) \
	do { \
		if (! tensor_valid(tensor)) { \
			syslog_error("Bad tensor."); \
			return RET_ERROR; \
		} \
	} while(0)

#define CHECK_TENSOR(tensor) \
	do { \
		if (! tensor_valid(tensor)) { \
			syslog_error("Bad tensor."); \
			return NULL; \
		} \
	} while(0)

	// Tensor
	typedef struct {
		DWORD magic;			// TENSOR_MAGIC
		WORD batch, chan, height, width;
		float *data; 			// TENSOR format is: BxCxHxW with float
	} TENSOR;

	int tensor_valid(TENSOR * tensor);
	TENSOR *tensor_create(WORD b, WORD c, WORD h, WORD w);
	TENSOR *tensor_copy(TENSOR * src);
	void tensor_destroy(TENSOR * tensor);
	void tensor_show(TENSOR * tensor);

	float *tensor_start_row(TENSOR *tensor, int b, int c, int h);
	float *tensor_start_chan(TENSOR *tensor, int b, int c);
	float *tensor_start_batch(TENSOR *tensor, int b);

	TENSOR *tensor_zoom(TENSOR *source, int nh, int nw);

	TENSOR *tensor_grid_sample(TENSOR *input, TENSOR *grid);
	TENSOR *tensor_slice_chan(TENSOR *tensor, int start, int stop);
	TENSOR *tensor_stack_chan(int n, TENSOR *tensor[]);

	int tensor_reshape(TENSOR *tensor, WORD nb, WORD nc, WORD nh, WORD nw);

#if defined(__cplusplus)
}
#endif
#endif							// _TENSOR_H
