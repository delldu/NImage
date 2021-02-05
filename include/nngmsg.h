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

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

int start_server(char *endpoint, nng_socket *socket);
int client_connect(char *endpoint, nng_socket *socket);

int request_send(nng_socket socket, int reqcode, TENSOR *tensor, float option);
TENSOR *request_recv(nng_socket socket, int *reqcode, float *option);

int response_send(nng_socket socket, TENSOR *tensor, int rescode);
TENSOR *response_recv(nng_socket socket, int *rescode);

#if defined(__cplusplus)
}
#endif
#endif							// _NNGMSG_H

