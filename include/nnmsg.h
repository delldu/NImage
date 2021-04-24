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

	#define HELLO_REQUEST_MESSAGE  0xFF01
	#define HELLO_RESPONSE_MESSAGE 0xFF02
	#define ERROR_TIMEOUT_MESSAGE 0xFF03
	#define OUTOF_SERVICE 0xFF04
	#define INVALID_SERVICE_MESSAGE 0xFF05

	int server_open(char *endpoint);
	int client_open(char *endpoint);

	int tensor_send(int socket, int reqcode, TENSOR * tensor);
	TENSOR *tensor_recv(int socket, int *reqcode);
	TENSOR *tensor_recv_timeout(int socket, int timeout, int *rescode);

	TENSOR *service_request(int socket, int *reqcode);
	int service_response(int socket, int msgcode, TENSOR *tensor);
	int service_avaible(int socket);

	int socket_readable(int socket, int timeout);

	void client_close(int socket);
	void server_close(int socket);

#if defined(__cplusplus)
}
#endif
#endif							// _NNMSG_H
