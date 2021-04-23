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

	#define HELLO_REQUEST_MESSAGE  0x9001
	#define HELLO_RESPONSE_MESSAGE 0x9002
	#define ERROR_TIMEOUT_MESSAGE 0x9003
	#define OUT_OF_SERVICE 0x0904
	#define INVALID_SERVICE_MESSAGE 0x0905

	int server_open(char *endpoint);
	int client_open(char *endpoint);

	int tensor_send(int socket, int reqcode, TENSOR * tensor);
	TENSOR *tensor_recv(int socket, int *reqcode);
	TENSOR *tensor_recv_timeout(int socket, int timeout, int *rescode);

	TENSOR *service_request_withcode(int socket, int *reqcode);
	TENSOR *service_request(int socket, int expected_msgcode);
	int service_response(int socket, int msgcode, TENSOR *tensor);
	int service_avaible(int socket);

	void client_close(int socket);
	void server_close(int socket);

#if defined(__cplusplus)
}
#endif
#endif							// _NNMSG_H
