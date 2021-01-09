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
			syslog_error("Bad tensor.\n"); \
			return RET_ERROR; \
		} \
	} while(0)

#define CHECK_TENSOR(tensor) \
	do { \
		if (! tensor_valid(tensor)) { \
			syslog_error("Bad tensor.\n"); \
			return NULL; \
		} \
	} while(0)


	// Support Tensor
	typedef struct {
		DWORD magic;		// TENSOR_MAGIC
		WORD batch, chan, height, width;
		DWORD opc;			// 4 bytes, opc is for RPC request/response
		BYTE *base;
	} TENSOR;

int tensor_valid(TENSOR *tensor);
TENSOR *tensor_create(WORD b, WORD c, WORD h, WORD w);
void tensor_destroy(TENSOR *tensor);

// xxxx8888
int tensor_abhead(TENSOR *tensor, BYTE *buffer);
TENSOR *tensor_fromab(BYTE *buf);
BYTE *tensor_toab(TENSOR *tensor);

#if defined(__cplusplus)
}
#endif

#endif	// _TENSOR_H

