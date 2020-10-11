/************************************************************************************
***
***	Copyright 2017-2020 Dell(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Tue Jul 25 02:41:54 PDT 2017
***
************************************************************************************/


#ifndef _HISTOGRAM_H
#define _HISTOGRAM_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "image.h"

#define HISTOGRAM_MAX_COUNT 256

typedef struct {
 	int total;
	int count[HISTOGRAM_MAX_COUNT];

	double cdf[HISTOGRAM_MAX_COUNT];
	
	int map[HISTOGRAM_MAX_COUNT];
} HISTOGRAM;

void histogram_reset(HISTOGRAM *h);
void histogram_add(HISTOGRAM *h, int c);
void histogram_del(HISTOGRAM *h, int c);
int histogram_middle(HISTOGRAM *h);
int histogram_top(HISTOGRAM *h, double ratio);
int histogram_clip(HISTOGRAM *h, int threshold);
int histogram_cdf(HISTOGRAM *h);
int histogram_map(HISTOGRAM *h, int max);
double histogram_likeness(HISTOGRAM *h1, HISTOGRAM *h2);
void histogram_sum(HISTOGRAM *sum, HISTOGRAM *sub);

int histogram_rect(HISTOGRAM *hist, IMAGE *img, RECT *rect);
void histogram_dump(HISTOGRAM *h);

#if defined(__cplusplus)
}
#endif

#endif	// _HISTOGRAM_H

