
/************************************************************************************
***
***	Copyright 2010-2020 Dell Du(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Sat Jul 31 14:19:59 HKT 2010
***
************************************************************************************/

#include "common.h"

static TIME __system_ms_time;

// return ms
TIME get_time()
{
	TIME ms;
	struct timeval t;

	gettimeofday(&t, NULL);
	ms = t.tv_sec * 1000 + t.tv_usec / 1000;

	return ms;
}

void time_reset()
{
	__system_ms_time = get_time();
}

void time_spend(char *prompt)
{
	printf("%s spend %ld ms.\n", prompt, get_time() - __system_ms_time);

	time_reset();
}

// example: maxhw == 512, times == 8
void resize(int h, int w, int maxhw, int times, int *nh, int *nw)
{
	int m;
	float scale;

	*nh = h; *nw = w;
	m = MAX(h, w);
	if (m > maxhw) {
		scale = 1.0 * maxhw / m;
		*nh = (int)(scale * h);
		*nw = (int)(scale * w);
	}

	*nh = times * (*nh / times);
	*nw = times * (*nw / times);
}