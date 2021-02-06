#ifndef _COMMON_H
#define _COMMON_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <arpa/inet.h>			// Suppot htonl, ntohl etc ...
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
#include <syslog.h>				// syslog, RFC3164 ?

// config.h
#define CONFIG_JPEG
#define CONFIG_PNG

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
#define MIN_FLOAT_NUMBER 0.000001f

// Zoom method
#define ZOOM_METHOD_COPY 0
#define ZOOM_METHOD_BLINE 1


#define CheckPoint(fmt, arg...) printf("# CheckPoint: %d(%s): " fmt "\n", (int)__LINE__, __FILE__, ##arg)
#if 0
#define syslog_info(fmt, arg...)  do { \
			syslog(LOG_INFO, "Info: %d(%s): " fmt "\n", (int)__LINE__, __FILE__, ##arg); \
		} while (0)
#define syslog_debug(fmt, arg...)  do { \
			syslog(LOG_DEBUG, "Debug: %d(%s): " fmt "\n", (int)__LINE__, __FILE__, ##arg); \
		} while (0)
#define syslog_error(fmt, arg...)  do { \
			syslog(LOG_ERR, "Error: %d(%s): " fmt "\n", (int)__LINE__, __FILE__, ##arg); \
		} while (0)
#else
#define syslog_info(fmt, arg...)  do { \
		fprintf(stderr, "Info: %d(%s): " fmt "\n", (int)__LINE__, __FILE__, ##arg); \
	} while (0)
#define syslog_debug(fmt, arg...)  do { \
		fprintf(stderr, "Debug: %d(%s): " fmt "\n", (int)__LINE__, __FILE__, ##arg); \
	} while (0)
#define syslog_error(fmt, arg...)  do { \
		fprintf(stderr, "Error: %d(%s): " fmt "\n", (int)__LINE__, __FILE__, ##arg); \
	} while (0)
#endif

#define ARRAY_SIZE(x) (int)(sizeof(x)/sizeof(x[0]))

// Big Enddian ..
#define MAKE_FOURCC(a,b,c,d) (((DWORD)(a) << 24) | ((DWORD)(b) << 16) | ((DWORD)(c) << 8) | ((DWORD)(d) << 0))
#define GET_FOURCC1(abcd) ((BYTE)(((abcd) >> 24) & 0xff))
#define GET_FOURCC2(abcd) ((BYTE)(((abcd) >> 16) & 0xff))
#define GET_FOURCC3(abcd) ((BYTE)(((abcd) >> 8) & 0xff))
#define GET_FOURCC4(abcd) ((BYTE)(((abcd) >> 0) & 0xff))

#define MAKE_TWOCC(a,b) (((DWORD)(a) << 8) | ((DWORD)(b) << 0))
#define GET_TWOCC1(ab) ((BYTE)(((ab) >> 8) & 0xff))
#define GET_TWOCC2(ab) ((BYTE)(((ab) >> 0) & 0xff))


	typedef struct {
		int r, c, h, w;
	} RECT;
#define rect_foreach(rect, i, j) \
	for (i = 0; i < (rect)->h; i++) \
		for (j = 0; j < (rect)->w; j++)

// Time
	void time_reset();
	void time_spend(char *prompt);


#if defined(__cplusplus)
}
#endif
#endif							// _COMMON_H
