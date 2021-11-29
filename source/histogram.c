/************************************************************************************
***
***	Copyright 2017-2020 Dell(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Thu Jul 20 00:40:34 PDT 2017
***
************************************************************************************/
#include "image.h"

void histogram_reset(HISTOGRAM * h)
{
	h->total = 0;
	memset(h->count, 0, HISTOGRAM_MAX_COUNT * sizeof(int));
	memset(h->cdf, 0, HISTOGRAM_MAX_COUNT * sizeof(float));
	memset(h->map, 0, HISTOGRAM_MAX_COUNT * sizeof(int));
}

void histogram_add(HISTOGRAM * h, int c)
{
	c = MAX(c, 0);
	c = MIN(c, HISTOGRAM_MAX_COUNT - 1);
	h->count[c]++;
	h->total++;
}

void histogram_del(HISTOGRAM * h, int c)
{
	c = MAX(c, 0);
	c = MIN(c, HISTOGRAM_MAX_COUNT - 1);

	h->count[c]--;
	h->total--;
}

int histogram_middle(HISTOGRAM * h)
{
	int i, halfsum, sum;

	sum = 0;
	halfsum = h->total / 2;
	for (i = 0; i < HISTOGRAM_MAX_COUNT; i++) {
		sum += h->count[i];
		if (sum >= halfsum)
			return i;
	}

	// next is not impossiable
	return HISTOGRAM_MAX_COUNT / 2;	// (0 + 255)/2
}

int histogram_top(HISTOGRAM * h, float ratio)
{
	int i, threshold, sum;

	sum = 0;
	threshold = (int) (ratio * h->total);
	threshold = MAX(threshold, 1);

	for (i = HISTOGRAM_MAX_COUNT - 1; i >= 0; i--) {
		sum += h->count[i];
		if (sum >= threshold) {
			return i;
		}
	}

	return 0;
}

int histogram_clip(HISTOGRAM * h, int threshold)
{
	int i, step, excess, upper, binavg;

	excess = 0;
	for (i = 0; i < HISTOGRAM_MAX_COUNT; i++) {
		if (h->count[i] > threshold) {
			excess += h->count[i] - threshold;
		}
	}

	binavg = excess / HISTOGRAM_MAX_COUNT;
	upper = threshold - binavg;
	for (i = 0; i < HISTOGRAM_MAX_COUNT; i++) {
		if (h->count[i] > threshold) {
			h->count[i] = threshold;
		} else {
			if (h->count[i] > upper) {
				excess -= (threshold - h->count[i]);
				h->count[i] = threshold;
			} else {
				h->count[i] += binavg;
				excess -= binavg;
			}
		}
	}

	while (excess > 0) {
		step = HISTOGRAM_MAX_COUNT / excess;
		if (step < 1)
			step = 1;

		for (i = 0; i < HISTOGRAM_MAX_COUNT; i += step) {
			h->count[i]++;
			excess--;
		}
	}

	return RET_OK;
}

int histogram_cdf(HISTOGRAM * h)
{
	int i;
	float sum = 0;

	if (h->total < 1)
		return RET_ERROR;
	for (i = 0; i < HISTOGRAM_MAX_COUNT; i++) {
		sum += h->count[i];
		h->cdf[i] = sum / (float) (h->total);
	}
	return RET_OK;
}

int histogram_map(HISTOGRAM * h, int max)
{
	int i;

	for (i = 0; i < HISTOGRAM_MAX_COUNT; i++) {
		h->map[i] = (int) (h->cdf[i] * max + 0.5);
		if (h->map[i] > max)
			h->map[i] = max;
	}
	return RET_OK;
}

// Suppose: image is gray
int histogram_rect(HISTOGRAM * hist, IMAGE * img, RECT * rect)
{
	int i, j;
	BYTE n;
	check_image(img);

	image_rectclamp(img, rect);
	histogram_reset(hist);
	for (i = rect->r; i < rect->r + rect->h; i++) {
		for (j = rect->c; j < rect->c + rect->w; j++) {
			color_rgb2gray(img->ie[i][j].r, img->ie[i][j].g, img->ie[i][j].b, &n);
			histogram_add(hist, n);
		}
	}
	return RET_OK;
}

float histogram_likeness(HISTOGRAM * h1, HISTOGRAM * h2)
{
	int k;
	float d, sum;

	sum = 0.0;
	for (k = 0; k < HISTOGRAM_MAX_COUNT; k++) {
		d = (float) h1->count[k] / (float) h1->total * (float) h2->count[k] / (float) h2->total;
		d = sqrt(d);
		sum += d;
	}

	return sum;
}

void histogram_sum(HISTOGRAM * sum, HISTOGRAM * sub)
{
	int k;

	for (k = 0; k < HISTOGRAM_MAX_COUNT; k++) {
		sum->count[k] += sub->count[k];
	}
	sum->total += sub->total;
}

void histogram_dump(HISTOGRAM * h)
{
	int i;

	printf("Histogram:\n");
	for (i = 0; i < HISTOGRAM_MAX_COUNT; i++) {
		if (h->count[i] < 1)
			continue;

		printf("%3d, count: %6d(%10.4f %%), cdf: %10.4f(%5d), map: %3d\n", i,
			   h->count[i], (float) (100.0 * h->count[i]) / (float) h->total,
			   h->cdf[i], (int) (h->cdf[i] * h->total), h->map[i]);
	}
}
