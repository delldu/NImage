/************************************************************************************
***
***	Copyright 2021 Dell(18588220928g@163.com), All Rights Reserved.
***
***	File Author: Dell, 2021-01-09 14:44:13
***
************************************************************************************/

#ifndef _NNMSG_H
#define _NNMSG_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "tensor.h"

	int server_open(char *endpoint);
	int client_open(char *endpoint);

	int request_send(int socket, int reqcode, TENSOR * tensor, float option);
	TENSOR *request_recv(int socket, int *reqcode, float *option);

	int response_send(int, TENSOR * tensor, int rescode);
	TENSOR *response_recv(int socket, int *rescode);

	void client_close(int socket);
	void server_close(int socket);

#if defined(__cplusplus)
}
#endif
#endif							// _NNMSG_H