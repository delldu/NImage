#ifndef _COMMON_H
#define _COMMON_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>

#define BYTE uint8_t
#define WORD uint16_t
#define DWORD uint32_t
#define TIME int64_t
#define HASH64 uint64_t

#define RET_FAIL 1
#define RET_OK 0
#define RET_ERROR (-1)

#define MIN(a, b)  ((a) > (b)? (b) : (a))
#define MAX(a, b)  ((a) > (b)? (a) : (b))
#define CLAMP(x,min,max) ((x) < (min) ? (min) : (x) > (max) ? (max) : (x))
#define ABS(a) ((a) > 0? (a) : (-(a)))

#define MATH_PI 3.1415926f
#define MIN_DOUBLE_NUMBER 0.000001f

// Zoom method
#define ZOOM_METHOD_COPY 0
#define ZOOM_METHOD_BLINE 1


#define CheckPoint(fmt, arg...) printf("# CheckPoint: %d(%s): " fmt "\n", (int)__LINE__, __FILE__, ##arg)
#define syslog_print(fmt, arg...) fprintf(stderr, fmt, ##arg)
#define syslog_debug(fmt, arg...)  do { \
		fprintf(stdout, "Debug: %d(%s): " fmt "\n", (int)__LINE__, __FILE__, ##arg); \
	} while (0)
#define syslog_error(fmt, arg...)  do { \
		fprintf(stderr, "Error: %d(%s): " fmt "\n", (int)__LINE__, __FILE__, ##arg); \
		exit(0); \
	} while (0)

#define ARRAY_SIZE(x) (int)(sizeof(x)/sizeof(x[0]))

#define MAKE_FOURCC(a,b,c,d) (((DWORD)(a) << 0) | ((DWORD)(b) << 8) | ((DWORD)(c) << 16) | ((DWORD)(d) << 24))
#define GET_FOURCC1(a) ((BYTE)((a) & 0xff))
#define GET_FOURCC2(a) ((BYTE)(((a)>>8) & 0xff))
#define GET_FOURCC3(a) ((BYTE)(((a)>>16) & 0xff))
#define GET_FOURCC4(a) ((BYTE)(((a)>>24) & 0xff))


typedef struct { int r, c, h, w; } RECT;
#define rect_foreach(rect, i, j) \
	for (i = 0; i < (rect)->h; i++) \
		for (j = 0; j < (rect)->w; j++)

// Time
void time_reset();
void time_spend(char *prompt);


#if defined(__cplusplus)
}
#endif

#endif	// _COMMON_H


