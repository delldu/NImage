/************************************************************************************
***
***	Copyright 2021 Dell(18588220928g@163.com), All Rights Reserved.
***
***	File Author: Dell, 2021-01-09 14:44:13
***
************************************************************************************/

#ifndef _NNGMSG_H
#define _NNGMSG_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "tensor.h"

	int start_server(char *endpoint);
	int client_connect(char *endpoint);

	int request_send(int socket, int reqcode, TENSOR * tensor, float option);
	TENSOR *request_recv(int socket, int *reqcode, float *option);

	int response_send(int, TENSOR * tensor, int rescode);
	TENSOR *response_recv(int socket, int *rescode);

#if defined(__cplusplus)
}
#endif
#endif							// _NNGMSG_H
