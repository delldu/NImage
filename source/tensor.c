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

int tensor_valid(TENSOR *tensor) {
  return (!tensor || tensor->batch < 1 || tensor->chan < 1 ||
          tensor->height < 1 || tensor->width < 1 ||
          tensor->magic != TENSOR_MAGIC)
             ? 0
             : 1;
}

TENSOR *tensor_create(int b, int c, int h, int w) {
  TENSOR *t;

  BYTE *base =
      (BYTE *)calloc((size_t)1, sizeof(TENSOR) + b * c * h * w * sizeof(float));
  if (!base) {
    syslog_error("Allocate memeory.");
    return NULL;
  }
  t = (TENSOR *)base;

  t->magic = TENSOR_MAGIC;
  t->batch = b;
  t->chan = c;
  t->height = h;
  t->width = w;
  t->data = (float *)(base + sizeof(TENSOR));

  return t;
}

int tensor_zero_(TENSOR *tensor) {
  int n;
  check_tensor(tensor);
  n = tensor->batch * tensor->chan * tensor->height * tensor->width;
  memset(tensor->data, 0, n * sizeof(float));

  return RET_OK;
}

int tensor_clamp_(TENSOR *tensor, float low, float high) {
  int i, n;
  float d;
  check_tensor(tensor);
  n = tensor->batch * tensor->chan * tensor->height * tensor->width;
  for (i = 0; i < n; i++) {
    d = CLAMP(tensor->data[i], low, high);
    tensor->data[i] = d;
  }
  return RET_OK;
}

TENSOR *tensor_copy(TENSOR *src) {
  TENSOR *dst;

  CHECK_TENSOR(src);

  dst = tensor_create(src->batch, src->chan, src->height, src->width);
  if (tensor_valid(dst)) {
    memcpy(dst->data, src->data,
           src->batch * src->chan * src->height * src->width * sizeof(float));
  }

  return dst;
}

void tensor_show(TENSOR *tensor) {
  int i, n, show_numbers = 10;

  syslog_info("Tensor: %dx%dx%dx%d", tensor->batch, tensor->chan,
              tensor->height, tensor->width);
  n = tensor->batch * tensor->chan * tensor->height * tensor->width;
  for (i = 0; i < show_numbers; i++) {
    printf("%.4f ", tensor->data[i]);
  }
  printf("... ");
  for (i = n - show_numbers; i < n; i++) {
    printf("%.4f ", tensor->data[i]);
  }
  printf("\n");
}

void tensor_destroy(TENSOR *tensor) {
  if (!tensor_valid(tensor))
    return;

  free(tensor);
}

IMAGE *image_from_tensor(TENSOR *tensor, int k) {
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

  image_foreach(image, i, j) { image->ie[i][j].r = (BYTE)((*R++) * 255); }

  if (tensor->chan >= 2) {
    image_foreach(image, i, j) image->ie[i][j].g = (BYTE)((*G++) * 255);
  }

  if (tensor->chan >= 3) {
    image_foreach(image, i, j) image->ie[i][j].b = (BYTE)((*B++) * 255);
  }

  if (tensor->chan >= 4) {
    image_foreach(image, i, j) image->ie[i][j].a = (BYTE)((*A++) * 255);
  }

  return image;
}

TENSOR *tensor_from_image(IMAGE *image, int with_alpha) {
  int i, j;
  TENSOR *tensor;
  float *R, *G, *B, *A = NULL;

  CHECK_IMAGE(image);

  if (with_alpha)
    tensor = tensor_create(1, sizeof(RGBA_8888), image->height, image->width);
  else
    tensor = tensor_create(1, 3, image->height, image->width); // RGB
  CHECK_TENSOR(tensor);

  R = tensor->data;
  G = R + tensor->height * tensor->width;
  B = G + tensor->height * tensor->width;
  if (with_alpha)
    A = B + tensor->height * tensor->width;

  image_foreach(image, i, j) {
    *R++ = (float)image->ie[i][j].r / 255.0;
    *G++ = (float)image->ie[i][j].g / 255.0;
    *B++ = (float)image->ie[i][j].b / 255.0;
  }
  if (with_alpha) {
    image_foreach(image, i, j) *A++ = (float)image->ie[i][j].a / 255.0;
  }

  return tensor;
}

float *tensor_start_row(TENSOR *tensor, int b, int c, int h) {
  int offset;
  if (b < 0 || b >= tensor->batch || c < 0 || c >= tensor->chan || h < 0 ||
      h >= tensor->height)
    return NULL;
  offset = b * tensor->chan * tensor->height * tensor->width +
           c * tensor->height * tensor->width + h * tensor->width;
  return tensor->data + offset;
}

float *tensor_start_chan(TENSOR *tensor, int b, int c) {
  int offset;
  if (b < 0 || b >= tensor->batch || c < 0 || c >= tensor->chan)
    return NULL;
  offset = b * tensor->chan * tensor->height * tensor->width +
           c * tensor->height * tensor->width;
  return tensor->data + offset;
}

float *tensor_start_batch(TENSOR *tensor, int b) {
  int offset;
  if (b < 0 || b >= tensor->batch)
    return NULL;
  offset = b * tensor->chan * tensor->height * tensor->width;
  return tensor->data + offset;
}

TENSOR *tensor_zoom(TENSOR *source, int nh, int nw) {
  int b, c;
  MATRIX *s_mat, *d_mat;
  float *s_data, *d_data;
  TENSOR *zoom = NULL;

  CHECK_TENSOR(source);
  zoom = tensor_create(source->batch, source->chan, nh, nw);
  CHECK_TENSOR(zoom);

  s_mat = matrix_create(source->height, source->width);
  CHECK_MATRIX(s_mat);
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

TENSOR *tensor_zeropad(TENSOR *source, int nh, int nw) {
  int b, c, i, j;
  float *s_row, *d_row;
  TENSOR *destion = NULL;

  CHECK_TENSOR(source);
  destion = tensor_create(source->batch, source->chan, nh, nw);
  CHECK_TENSOR(destion);

  for (b = 0; b < source->batch; b++) {
    for (c = 0; c < source->chan; c++) {
      for (i = 0; i < source->height && i < nh; i++) {
        s_row = tensor_start_row(source, b, c, i);
        d_row = tensor_start_row(destion, b, c, i);
        for (j = 0; j < source->width && j < nw; j++)
          d_row[j] = s_row[j];
      }
    }
  }

  return destion;
}

TENSOR *tensor_rgb2lab(IMAGE *image) {
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
    color_rgb2lab(image->ie[i][j].r, image->ie[i][j].g, image->ie[i][j].b, &L,
                  &a, &b);
    L = (L - 50.f) / 100.f;
    a /= 110.f;
    b /= 110.f;
    *R++ = L;
    *G++ = a;
    *B++ = b;
    *A++ = (image->ie[i][j].a > 127) ? 1.0 : 0.f;
  }

  return tensor;
}

IMAGE *tensor_lab2rgb(TENSOR *tensor, int k) {
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

  image = image_create(tensor->height, tensor->width);
  CHECK_IMAGE(image);
  R = tensor->data + k * (tensor->chan * tensor->height * tensor->width);
  G = R + tensor->height * tensor->width;
  B = G + tensor->height * tensor->width;
  A = B + tensor->height * tensor->width;

  image_foreach(image, i, j) {
    L = *R++;
    a = *G++;
    b = *B++;
    L = L * 100.f + 50.f;
    a *= 110.f;
    b *= 110.f;
    color_lab2rgb(L, a, b, &r0, &g0, &b0);
    image->ie[i][j].r = r0;
    image->ie[i][j].g = g0;
    image->ie[i][j].b = b0;
    image->ie[i][j].a = 255;
  }

  if (tensor->chan >= 4) {
    image_foreach(image, i, j) image->ie[i][j].a = (BYTE)((*A++) * 255);
  }

  return image;
}

int tensor_setmask(TENSOR *tensor, float mask) {
  int i, n, b;
  float *alpha;

  check_tensor(tensor);
  if (tensor->chan != 2 && tensor->chan != 4)
    return RET_OK;
  n = tensor->height * tensor->width;
  for (b = 0; b < tensor->batch; b++) {
    alpha =
        tensor_start_chan(tensor, b /*batch*/, tensor->chan - 1 /*channel*/);
    for (i = 0; i < n; i++)
      *alpha++ = mask;
  }
  return RET_OK;
}

/********************************************************************************
 *
 *	Make Grid Tensor:
 *
 *   return:
 *      Bx2xHxW Tensor, (-1, -1)
 *
 *********************************************************************************/
TENSOR *tensor_make_grid(int batch, int height, int width) {
  int i, j, b;
  float *imap, *jmap, d, delta;
  TENSOR *grid;

  grid = tensor_create(batch, 2, height, width);
  CHECK_TENSOR(grid);

  for (b = 0; b < grid->batch; b++) {
    imap = tensor_start_chan(grid, b, 0);
    jmap = tensor_start_chan(grid, b, 1);

    /*****************************************************
     * Imap:
     *   0				0
     *   1				1
     *   2				2
     * ...				...
     * 512				512
     *****************************************************/
    delta = 2.0 / grid->height;
    d = -1.0 + delta / 2.0; // First
    for (i = 0; i < grid->height; i++) {
      for (j = 0; j < grid->width; j++)
        *imap++ = d; // same cols
      d += delta;
    }
    /*****************************************************
     * Jmap:
     *   0	1	2	...		960
     *   0	1	2	...		960
     *   0	1	2	...		960
     *   0	1	2	...		960
     *****************************************************/
    delta = 2.0 / grid->width;
    for (i = 0; i < grid->height; i++) {
      d = -1.0 + delta / 2.0; // First
      for (j = 0; j < grid->width; j++) {
        *jmap++ = d;
        d += delta;
      }
    }
  }

  return grid;
}

/********************************************************************************
 *
 *	Grid Sample:
 *
 *   Input with shape (B, C, H_in, W_in)
 *   Grid with shape (B, 2, H_out, W_out), values according suppose X in
 *		int matrix_sample(MATRIX * mat, MATRIX *imap, MATRIX *jmap, MATRIX
 **output_mat);
 *
 *   Output will have shape (B, C, H_out, W_out)
 *
 *********************************************************************************/

TENSOR *tensor_grid_sample(TENSOR *input, TENSOR *grid) {
  int b, c, n;
  float *input_d, *output_d, *grid_d;
  MATRIX *input_mat, *output_mat, *grid_imap, *grid_jmap;
  TENSOR *output;

  CHECK_TENSOR(input);
  CHECK_TENSOR(grid);

  if (grid->chan != 2) {
    syslog_error("Grid must Bx2xHxW tensor.");
    return NULL;
  }

  // Create temp matrixs for sample
  input_mat = matrix_create(input->height, input->width);
  CHECK_MATRIX(input_mat);
  output_mat = matrix_create(grid->height, grid->width);
  CHECK_MATRIX(output_mat);
  grid_imap = matrix_create(grid->height, grid->width);
  CHECK_MATRIX(grid_imap);
  grid_jmap = matrix_create(grid->height, grid->width);
  CHECK_MATRIX(grid_jmap);

  output = tensor_create(input->batch, input->chan, grid->height, grid->width);
  CHECK_TENSOR(output);

  n = grid->height * grid->width;
  for (b = 0; b < output->batch; b++) {
    for (c = 0; c < output->chan; c++) {
      // Prepare grid i, j
      grid_d = tensor_start_chan(grid, b, 0);
      memcpy(grid_imap->base, grid_d, n * sizeof(float));
      grid_d = tensor_start_chan(grid, b, 1);
      memcpy(grid_jmap->base, grid_d, n * sizeof(float));

      // Create source data
      input_d = tensor_start_chan(input, b, c);
      memcpy(input_mat->base, input_d,
             input->height * input->width * sizeof(float));

      // Sampling ...
      matrix_sample(input_mat, grid_imap, grid_jmap, output_mat);

      // Save destion data
      output_d = tensor_start_chan(output, b, c);
      memcpy(output_d, output_mat->base, n * sizeof(float));
    }
  }

  matrix_destroy(grid_jmap);
  matrix_destroy(grid_imap);
  matrix_destroy(output_mat);
  matrix_destroy(input_mat);

  return output;
}

TENSOR *tensor_make_cell(int batch, int height, int width) {
  int i, j, b;
  float *imap, *jmap, d;
  TENSOR *cell;

  cell = tensor_create(batch, 2, height, width);
  CHECK_TENSOR(cell);

  for (b = 0; b < cell->batch; b++) {
    imap = tensor_start_chan(cell, b, 0);
    jmap = tensor_start_chan(cell, b, 1);

    d = 2.0 / cell->height;
    for (i = 0; i < cell->height; i++) {
      for (j = 0; j < cell->width; j++)
        *imap++ = d;
    }

    d = 2.0 / cell->width;
    for (i = 0; i < cell->height; i++) {
      for (j = 0; j < cell->width; j++)
        *jmap++ = d;
    }
  }

  return cell;
}

TENSOR *tensor_flow_backwarp(TENSOR *image, TENSOR *flow) {
  int i, j, b;
  float *imap, *jmap, *uflow, *vflow;
  TENSOR *grid, *output;

  CHECK_TENSOR(image);
  CHECK_TENSOR(flow);

  if (flow->chan != 2) {
    syslog_error("Flow must be Bx2xHxW tensor.");
    return NULL;
  }

  grid = tensor_create(flow->batch, 2, flow->height, flow->width);
  CHECK_TENSOR(grid);

  for (b = 0; b < grid->batch; b++) {
    imap = tensor_start_chan(grid, b, 0);
    jmap = tensor_start_chan(grid, b, 1);

    uflow = tensor_start_chan(flow, b, 0);
    vflow = tensor_start_chan(flow, b, 1);

    /*****************************************************
     * Imap:
     *   0				0
     *   1				1
     *   2				2
     * ...				...
     * 512				512
     *****************************************************/
    for (i = 0; i < grid->height; i++) {
      for (j = 0; j < grid->width; j++)
        *imap++ = (i + *vflow++) / grid->height; // same cols
    }
    /*****************************************************
     * Jmap:
     *   0	1	2	...		960
     *   0	1	2	...		960
     *   0	1	2	...		960
     *   0	1	2	...		960
     *****************************************************/
    for (i = 0; i < grid->height; i++) {
      for (j = 0; j < grid->width; j++)
        *jmap++ = (j + *uflow++) / grid->width; // same rows
    }
  }

  output = tensor_grid_sample(image, grid);

  tensor_destroy(grid);

  return output;
}

int tensor_view_(TENSOR *tensor, int nb, int nc, int nh, int nw) {
  check_tensor(tensor);

  // Capacity is same ...
  if (nb * nc * nh * nw ==
      tensor->batch * tensor->chan * tensor->height * tensor->width) {
    tensor->batch = nb;
    tensor->chan = nc;
    tensor->height = nh;
    tensor->width = nw;

    return RET_OK;
  }

  // Different capacity ...
  return RET_ERROR;
}

TENSOR *tensor_reshape(TENSOR *tensor, int nb, int nc, int nh, int nw) {
  int b, c, n;
  float *src, *dst;
  TENSOR *zoom, *output;

  CHECK_TENSOR(tensor);
  if (nb < 1 || nc < 1 || nh < 1 || nw < 1)
    return NULL;

  // Capacity is same ...
  if (nb * nc * nh * nw ==
      tensor->batch * tensor->chan * tensor->height * tensor->width) {
    output = tensor_copy(tensor);
    CHECK_TENSOR(output);
    output->batch = nb;
    output->chan = nc;
    output->height = nh;
    output->width = nw;
    return output;
  }

  // Different capacity ...
  if (tensor->height == nh && tensor->width == nw)
    zoom = tensor_copy(tensor);
  else
    zoom = tensor_zoom(tensor, nh, nw);
  CHECK_TENSOR(zoom);

  if (zoom->batch == nb && zoom->chan == nc)
    return zoom;

  // zoom->batch != nb || zoom->channel != nc
  output = tensor_create(nb, nc, nh, nw);
  CHECK_TENSOR(output);

  n = output->height * output->width;
  for (b = 0; b < output->batch; b++) {
    for (c = 0; c < output->chan; c++) {
      src = tensor_start_chan(zoom, MIN(b, zoom->batch - 1),
                              MIN(c, zoom->chan - 1));
      dst = tensor_start_chan(output, b, c);
      memcpy(dst, src, n * sizeof(float));
    }
  }

  tensor_destroy(zoom);

  return output;
}

extern int math_gsbw(float sigma);
extern int matrix_gauss_filter(MATRIX *mat, float sigma);

// sigma == 0.5, window size is 5x5
int tensor_dilate_smooth(TENSOR *tensor, float sigma) {
  MATRIX *mat;
  float *from, *to, d;
  int i, j, i2, j2, b, c, n, radius;

  check_tensor(tensor);
  mat = matrix_create(tensor->height, tensor->width);
  check_matrix(mat);
  n = tensor->height * tensor->width;
  radius = math_gsbw(sigma) / 2;

  for (b = 0; b < tensor->batch; b++) {
    for (c = 0; c < tensor->chan; c++) {
      from = tensor_start_chan(tensor, b, c);
      to = mat->base;

      // matrix get data from tensor
      memcpy(to, from, n * sizeof(float));

      // Dilate
      for (i = radius; i < mat->m - radius; i++) {
        for (j = radius; j < mat->n - radius; j++) {
          if (mat->me[i][j] < 0.5)
            continue;
          d = mat->me[i][j];
          for (i2 = i - radius; i2 <= i + radius; i2++) {
            for (j2 = j - radius; j2 <= j + radius; j2++)
              mat->me[i2][j2] = d;
          }
        }
      }

      matrix_gauss_filter(mat, sigma);

      // save matrix data to tensor
      memcpy(from, to, n * sizeof(float));
    }
  }
  matrix_destroy(mat);

  return RET_OK;
}

// [start, stop), for example: [0, 2)
TENSOR *tensor_slice_chan(TENSOR *tensor, int start, int stop) {
  int b, c, n;
  float *from, *to;
  TENSOR *output;

  CHECK_TENSOR(tensor);
  if (start >= tensor->chan || start >= stop)
    return NULL;

  output =
      tensor_create(tensor->batch, stop - start, tensor->height, tensor->width);
  CHECK_TENSOR(output);

  n = tensor->height * tensor->width;
  for (b = 0; b < tensor->batch; b++) {
    for (c = start; c < stop && c < tensor->chan; c++) {
      from = tensor_start_chan(tensor, b, c);
      to = tensor_start_chan(output, b, c - start);
      memcpy(to, from, n * sizeof(float));
    }
  }

  return output;
}

TENSOR *tensor_stack_chan(int n, TENSOR *tensors[]) {
  int i, b, len;
  float *from, *to;
  TENSOR *output;

  len = 0;
  for (i = 0; i < n; i++) {
    CHECK_TENSOR(tensors[i]);
    if (i >= 1) {
      if (tensors[i]->batch != tensors[0]->batch ||
          tensors[i]->height != tensors[0]->height ||
          tensors[i]->width != tensors[0]->width) {
        syslog_error(
            "Tensor batch, height or width is not same, so can not stack.");
        return NULL;
      }
    }
    len += tensors[i]->chan;
  }

  output = tensor_create(tensors[0]->batch, len, tensors[0]->height,
                         tensors[0]->width);
  CHECK_TENSOR(output);

  for (b = 0; b < output->batch; b++) {
    to = tensor_start_batch(output, b);
    for (i = 0; i < n; i++) {
      from = tensor_start_batch(tensors[i], b);
      len = tensors[i]->chan * tensors[i]->height * tensors[i]->width;
      memcpy(to, from, len * sizeof(float));
      to += len;
    }
  }

  return output;
}

// [start, stop), for example: [0, 2)
TENSOR *tensor_slice_row(TENSOR *tensor, int start, int stop) {
  int b, c, h, n;
  float *from, *to;
  TENSOR *output;

  CHECK_TENSOR(tensor);
  if (start >= tensor->height || start >= stop)
    return NULL;

  output =
      tensor_create(tensor->batch, tensor->chan, stop - start, tensor->width);
  CHECK_TENSOR(output);

  n = tensor->width;
  for (b = 0; b < tensor->batch; b++) {
    for (c = 0; c < tensor->chan; c++) {
      for (h = start; h < stop && h < tensor->height; h++) {
        from = tensor_start_row(tensor, b, c, h);
        to = tensor_start_row(output, b, c, h - start);
        memcpy(to, from, n * sizeof(float));
      }
    }
  }

  return output;
}

TENSOR *tensor_stack_row(int n, TENSOR *tensors[]) {
  int i, b, c, len;
  float *from, *to;
  TENSOR *output;

  len = 0;
  for (i = 0; i < n; i++) {
    CHECK_TENSOR(tensors[i]);
    if (i >= 1) {
      if (tensors[i]->batch != tensors[0]->batch ||
          tensors[i]->chan != tensors[0]->chan ||
          tensors[i]->width != tensors[0]->width) {
        syslog_error(
            "Tensor batch, channel or width is not same, so can not stack.");
        return NULL;
      }
    }
    len += tensors[i]->height;
  }

  output = tensor_create(tensors[0]->batch, tensors[0]->chan, len,
                         tensors[0]->width);
  CHECK_TENSOR(output);

  for (b = 0; b < output->batch; b++) {
    for (c = 0; c < output->chan; c++) {
      to = tensor_start_chan(output, b, c);
      for (i = 0; i < n; i++) {
        from = tensor_start_chan(tensors[i], b, c);
        len = tensors[i]->height * tensors[i]->width;
        memcpy(to, from, len * sizeof(float));
        to += len;
      }
    }
  }

  return output;
}
