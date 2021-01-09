/************************************************************************************
***
***	Copyright 2021 Dell(18588220928g@163.com), All Rights Reserved.
***
***	File Author: Dell, 2021-01-09 14:56:14
***
************************************************************************************/


#ifndef _ABHEAD_H
#define _ABHEAD_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "common.h"

//  Array Buffer
//  Array Buffer == AbHead + Data (BxCxHxW format)
	typedef struct {
		BYTE t[2];				// 2 bytes
		DWORD len;				// 4 bytes, data size
		WORD b, c, h, w;		// 8 bytes
		WORD opc;				// 2 bytes
		WORD crc;				// 2 bytes
	} AbHead;					// ArrayBuffer Head

	void abhead_init(AbHead * abhead);
	int valid_ab(BYTE * buf, size_t size);
	int abhead_decode(BYTE * buf, AbHead * head);
	int abhead_encode(AbHead * head, BYTE * buf);

#if defined(__cplusplus)
}
#endif
#endif							// _ABHEAD_H
