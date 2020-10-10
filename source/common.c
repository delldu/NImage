
/************************************************************************************
***
***	Copyright 2010-2020 Dell Du(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Sat Jul 31 14:19:59 HKT 2010
***
************************************************************************************/

#include "common.h"

#include <sys/wait.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>
#include <regex.h>
#include <sys/stat.h>
#include <fcntl.h>

static DOTS __global_dot_set;
static LINES __global_line_set;
static RECTS __global_rect_set;
static TIME __system_ms_time;

// Carte to polar coordinate
void math_topolar(int x, int y, double *r, double *a)
{
	double dr, da;

	if (x == 0) {
		if (y >= 0) {
			*a = 90; *r = y;
		}
		else {
			*a = 270; *r = -y;
		}
		return;
	}

	// x != 0
	dr = sqrt(x * x + y * y) + 0.5000;
	da = asin(1.0000 * y/dr)/3.1415926 * 180;

	*r = dr;
	if (x > 0) { //  (x, y) in 1/4 section
		*a = da;
		if (y < 0)
			*a += 360;
	}
	else  //  (x, y) in 2/3 section
		*a = 180 - da;
}


int math_arcindex(double a, int arcstep)
{
	int n;
	n = (2 * a + arcstep) / (2 * arcstep); // [a/arcstep + 1/2]
	if (n >= 360/arcstep)
		n = 0;
	return n;
}

// Guass band width
int math_gsbw(double sigma)
{
	int  dim;
	double d;

	d = 3 * sigma; dim = (int)d;
	if (d > (double)dim)
		dim++;

	return 2 * dim + 1;
}

DOTS *dot_set()
{
	return &__global_dot_set;
}

void dot_put(int r, int c)
{
	DOTS *dots = dot_set();
	if (dots->count < MAX_OBJECT_NUM) {
		dots->dot[dots->count].r = r;
		dots->dot[dots->count].c = c;
		dots->count++;
	}
	else {
		syslog_debug("Too many dots.");
	}
}

LINES *line_set()
{
	return &__global_line_set;
}

void line_put(int r1, int c1, int r2, int c2)
{
	LINES *lines = line_set();

	if (lines->count < MAX_OBJECT_NUM) {
		lines->line[lines->count].r1 = r1;
		lines->line[lines->count].c1 = c1;
		lines->line[lines->count].r2 = r2;
		lines->line[lines->count].c2 = c2;
		lines->count++;
	}
}

RECTS *rect_set()
{
	return &__global_rect_set;
}

int rect_put(RECT *rect)
{
	RECTS *mrs = rect_set();

	// Too small objects, ignore !
	if (rect->h < 8 || rect->w < 8)
		return RET_ERROR;

	if (20 * rect->h < rect->w || 20 * rect->w < rect->h) // Too narrow
		return RET_ERROR;

	if (mrs->count < MAX_OBJECT_NUM) {
		mrs->rect[mrs->count] = *rect;
		mrs->count++;
		return RET_OK;
	}

	return RET_ERROR;
}

int rect_delete(int i)
{
	RECTS *mrs = rect_set();

	if (i >= 0 && i < mrs->count) {
		mrs->count--;
		mrs->rect[i] = mrs->rect[mrs->count];
	
		return RET_OK;
	}

	return RET_ERROR;
}


// return ms
TIME get_time()
{
	TIME ms;
	struct timeval t;
	
	gettimeofday(&t, NULL);
	ms = t.tv_sec * 1000 + t.tv_usec/1000;

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

