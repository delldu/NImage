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

#define check_tensor(tensor)                                                   \
  do {                                                                         \
    if (!tensor_valid(tensor)) {                                               \
      syslog_error("Bad tensor.");                                             \
      return RET_ERROR;                                                        \
    }                                                                          \
  } while (0)

#define CHECK_TENSOR(tensor)                                                   \
  do {                                                                         \
    if (!tensor_valid(tensor)) {                                               \
      syslog_error("Bad tensor.");                                             \
      return NULL;                                                             \
    }                                                                          \
  } while (0)

// Tensor
typedef struct {
  DWORD magic; // TENSOR_MAGIC
  int batch, chan, height, width;
  float *data; // TENSOR format is: BxCxHxW with float
} TENSOR;

int tensor_valid(TENSOR *tensor);
TENSOR *tensor_create(int b, int c, int h, int w);
TENSOR *tensor_copy(TENSOR *src);
int tensor_zero_(TENSOR *tensor);
int tensor_clamp_(TENSOR *tensor, float low, float high);
void tensor_destroy(TENSOR *tensor);
void tensor_show(TENSOR *tensor);

float *tensor_start_row(TENSOR *tensor, int b, int c, int h);
float *tensor_start_chan(TENSOR *tensor, int b, int c);
float *tensor_start_batch(TENSOR *tensor, int b);

TENSOR *tensor_zoom(TENSOR *source, int nh, int nw);
TENSOR *tensor_zeropad(TENSOR *source, int nh, int nw);

TENSOR *tensor_make_grid(int batch, int height, int width);
TENSOR *tensor_grid_sample(TENSOR *input, TENSOR *grid);
TENSOR *tensor_flow_backwarp(TENSOR *image, TENSOR *flow);
TENSOR *tensor_make_cell(int batch, int height, int width);

TENSOR *tensor_slice_chan(TENSOR *tensor, int start, int stop);
TENSOR *tensor_stack_chan(int n, TENSOR *tensor[]);

TENSOR *tensor_slice_row(TENSOR *tensor, int start, int stop);
TENSOR *tensor_stack_row(int n, TENSOR *tensor[]);

int tensor_view_(TENSOR *tensor, int nb, int nc, int nh, int nw);
TENSOR *tensor_reshape(TENSOR *tensor, int nb, int nc, int nh, int nw);

int tensor_dilate_smooth(TENSOR *tensor, float sigma);

#if defined(__cplusplus)
}
#endif
#endif // _TENSOR_H
