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

	int tensor_send(int socket, int reqcode, TENSOR * tensor);
	TENSOR *tensor_recv(int socket, int *reqcode);
	TENSOR *tensor_recv_timeout(int socket, int timeout, int *rescode);

	void client_close(int socket);
	void server_close(int socket);

#if defined(__cplusplus)
}
#endif
#endif							// _NNMSG_H
