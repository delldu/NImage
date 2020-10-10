/************************************************************************************
***
***	Copyright 2017-2020 Dell(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Mon Aug  7 10:24:51 CST 2017
***
************************************************************************************/


#ifndef _HASH64_H
#define _HASH64_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "image.h"

#define HASH64 uint64_t

HASH64 image_ahash(IMAGE *image, char oargb, RECT *rect);
HASH64 image_phash(IMAGE *image, char oargb, RECT *rect);

HASH64 shape_hash(IMAGE *image, RECT *rect);
HASH64 texture_hash(IMAGE *image, RECT *rect);

int hash_hamming(HASH64 f1, HASH64 f2);

double hash_likeness(HASH64 f1, HASH64 f2);

void hash_dump(char *title, HASH64 finger);

// HASH Matrix
typedef struct {
	DWORD magic;
	int m, n, _m; 			// _m is internal rows
	HASH64 **me,*base; 
} HASHMAT;

#define hashmat_rect(rect, mat) \
	do { (rect)->r = 0; (rect)->c = 0; (rect)->h = mat->m; (rect)->w = mat->n; } while(0)

#define hashmat_foreach(mat, i, j) \
	for (i = 0; i < (mat)->m; i++) \
		for (j = 0; j < (mat)->n; j++)

#define check_hashmat(mat) \
	do { \
		if (! hashmat_valid(mat)) { \
			syslog_error("Bad matrix."); \
			return RET_ERROR; \
		} \
	} while(0)
		

#define CHECK_HASHMAT(mat) \
	do { \
		if (! hashmat_valid(mat)) { \
			syslog_error("Bad hashmat."); \
			return NULL; \
		} \
	} while(0)


int hashmat_valid(HASHMAT *M);
HASHMAT *hashmat_create(int m, int n);
void hashmat_print(HASHMAT *m);
void hashmat_destroy(HASHMAT *m);


#if defined(__cplusplus)
}
#endif

#endif	// _HASH64_H

