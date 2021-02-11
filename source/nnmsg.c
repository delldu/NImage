/************************************************************************************
***
***	Copyright 2021 Dell(18588220928g@163.com), All Rights Reserved.
***
***	File Author: Dell, 2021-01-09 14:43:18
***
************************************************************************************/

#include "nnmsg.h"

#include <msgpack.h>
#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>

#include <unistd.h>
#include <fcntl.h>

int server_open(char *endpoint)
{
	int socket;
	int buffer_size = 128*1024*1024;	// 128 M
	int max_recv_size = 256*1024*1024;	// 256 M

	// sudo journalctl -u image.service -n 10
	syslog_info("Start service on %s ...\n", endpoint);

    if ((socket = nn_socket(AF_SP, NN_REP)) < 0) {
	    syslog_error("nn_socket: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
	    return -1;
    }

	if (nn_setsockopt(socket, NN_SOL_SOCKET, NN_SNDBUF, &buffer_size, sizeof(buffer_size)) < 0) {
	    syslog_error("nn_setsockopt: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
	    return -1;
    }

	if (nn_setsockopt(socket, NN_SOL_SOCKET, NN_RCVBUF, &buffer_size, sizeof(buffer_size)) < 0) {
	    syslog_error("nn_setsockopt: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
	    return -1;
    }

	if (nn_setsockopt(socket, NN_SOL_SOCKET, NN_RCVMAXSIZE, &max_recv_size, sizeof(max_recv_size)) < 0) {
	    syslog_error("nn_setsockopt: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
	    return -1;
    }

    if (nn_bind(socket, endpoint) < 0) {
	    syslog_error("nn_bind: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
	    return -1;
    }

	syslog_info("Service already ...\n");

	return socket;
}

int client_open(char *endpoint)
{
	int socket;
	int buffer_size = 128*1024*1024;	// 128 M
	int max_recv_size = 256*1024*1024;	// 256 M
	int recv_timeout = 60 * 1000;		// 60 seconds

    // struct timeval timeout={5, 0};    // seconds
    if ((socket = nn_socket(AF_SP, NN_REQ)) < 0) {
	    syslog_error("nn_socket: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
	    return -1;
    }

	if (nn_setsockopt(socket, NN_SOL_SOCKET, NN_SNDBUF, &buffer_size, sizeof(buffer_size)) < 0) {
	    syslog_error("nn_setsockopt: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
	    return -1;
    }

	if (nn_setsockopt(socket, NN_SOL_SOCKET, NN_RCVBUF, &buffer_size, sizeof(buffer_size)) < 0) {
	    syslog_error("nn_setsockopt: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
	    return -1;
    }

	if (nn_setsockopt(socket, NN_SOL_SOCKET, NN_RCVMAXSIZE, &max_recv_size, sizeof(max_recv_size)) < 0) {
	    syslog_error("nn_setsockopt: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
	    return -1;
    }

	if (nn_setsockopt(socket, NN_SOL_SOCKET, NN_RCVTIMEO, &recv_timeout, sizeof(recv_timeout)) < 0) {
	    syslog_error("nn_setsockopt: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
	    return -1;
    }

    if (nn_connect(socket, endpoint) < 0) {
	    syslog_error("nn_connect: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
	    return -1;
    }

    return socket;
}


/****************************************************************************
* Request Tensor format:
*	int reqcode,
*	Tensor: uint16 [BxCxHxW], float [d1, ..., dn]
*	float option
****************************************************************************/
void request_encode(int reqcode, TENSOR * tensor, float option, msgpack_sbuffer *sbuf)
{
	int i, n;
	float *f;
	msgpack_packer pk;

	msgpack_packer_init(&pk, sbuf, msgpack_sbuffer_write);

	// Encode reqcode
	msgpack_pack_int(&pk, reqcode);

	// Encode tensor dims
	msgpack_pack_array(&pk, 4);
	{
		msgpack_pack_uint16(&pk, tensor->batch);
		msgpack_pack_uint16(&pk, tensor->chan);
		msgpack_pack_uint16(&pk, tensor->height);
		msgpack_pack_uint16(&pk, tensor->width);
	}

	// Encode tensor data
	f = tensor->data;
	n = tensor->batch * tensor->chan * tensor->height * tensor->width;
	msgpack_pack_array(&pk, n);
	{
		for (i = 0; i < n; i++)
			msgpack_pack_float(&pk, *f++);
	}

	// Encode option
	msgpack_pack_float(&pk, option);
}

int request_send(int socket, int reqcode, TENSOR * tensor, float option)
{
	int ret;
	msgpack_sbuffer sbuf;

	check_tensor(tensor);

	msgpack_sbuffer_init(&sbuf);
	request_encode(reqcode, tensor, option, &sbuf);

	// syslog_info("Request send ... size = %d", sbuf.size);
	if ((ret = nn_send(socket, sbuf.data, sbuf.size, 0)) < 0)
	    syslog_error("nn_send: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
	// syslog_info("Request send OK");

	msgpack_sbuffer_destroy(&sbuf);

	return (ret >= 0) ? RET_OK : RET_ERROR;
}

TENSOR *request_decode(BYTE *buf, int size, int *reqcode, float *option)
{
	int n;
	WORD dims[4];

	msgpack_unpacked msg;
	msgpack_unpack_return ret;
	size_t off = 0;
	TENSOR *tensor;
	float *f;

	msgpack_unpacked_init(&msg);

	// Decode reqcode
	*reqcode = 0;
	ret = msgpack_unpack_next(&msg, (char const *) buf, size, &off);
	if (ret == MSGPACK_UNPACK_SUCCESS) {
		msgpack_object obj = msg.data;
		if (obj.type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
			*reqcode = (int) obj.via.u64;
		} else if (obj.type == MSGPACK_OBJECT_NEGATIVE_INTEGER) {
			*reqcode = (int) obj.via.i64;
		}
	}
	// Decode tensor dims
	ret = msgpack_unpack_next(&msg, (char const *) buf, size, &off);
	if (ret == MSGPACK_UNPACK_SUCCESS) {
		msgpack_object obj = msg.data;
		if (obj.type == MSGPACK_OBJECT_ARRAY && obj.via.array.size != 0) {
			msgpack_object *p = obj.via.array.ptr;
			msgpack_object *const pend = obj.via.array.ptr + obj.via.array.size;
			for (n = 0; p < pend && n < 4; ++p, n++)
				dims[n] = (WORD) (p->via.i64);
		}
	}

	tensor = tensor_create(dims[0], dims[1], dims[2], dims[3]);
	CHECK_TENSOR(tensor);

	// Decode tensor data
	f = tensor->data;
	ret = msgpack_unpack_next(&msg, (char const *) buf, size, &off);
	if (ret == MSGPACK_UNPACK_SUCCESS) {
		msgpack_object obj = msg.data;
		if (obj.type == MSGPACK_OBJECT_ARRAY && obj.via.array.size != 0) {
			msgpack_object *p = obj.via.array.ptr;
			msgpack_object *const pend = obj.via.array.ptr + obj.via.array.size;
			for (; p < pend; ++p)
				*f++ = (float) (p->via.f64);
		}
	}
	// Decode tensor option
	*option = 0;
	ret = msgpack_unpack_next(&msg, (char const *) buf, size, &off);
	if (ret == MSGPACK_UNPACK_SUCCESS) {
		msgpack_object obj = msg.data;
		if (obj.type == MSGPACK_OBJECT_FLOAT32)
			*option = (float) obj.via.f64;
	}
	// Check buffer decode over status
	if (ret == MSGPACK_UNPACK_PARSE_ERROR) {
		syslog_error("The data in buf is invalid format.");
	}
	msgpack_unpacked_destroy(&msg);

	return tensor;
}

TENSOR *request_recv(int socket, int *reqcode, float *option)
{
	int size;
	BYTE *buf;
	TENSOR *tensor;

	if ((size = nn_recv(socket, &buf, NN_MSG, 0)) < 0) {
	    syslog_error("nn_recv: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
		return NULL;
	}

	syslog_info("Request tensor parsing ... ");
	tensor = request_decode(buf, size, reqcode, option);
	syslog_info("Response tensor parsed OK.");

	nn_freemsg(buf);		// Message has been saved ...

	return tensor;
}

/****************************************************************************
* Response Tensor format:
*	Tensor: uint16 [BxCxHxW], float [d1, ..., dn]
*	int rescode
****************************************************************************/
void response_encode(TENSOR * tensor, int rescode, msgpack_sbuffer *sbuf)
{
	int i, n;
	float *f;
	msgpack_packer pk;

	msgpack_packer_init(&pk, sbuf, msgpack_sbuffer_write);

	// Encode tensor dims
	msgpack_pack_array(&pk, 4);
	{
		msgpack_pack_uint16(&pk, tensor->batch);
		msgpack_pack_uint16(&pk, tensor->chan);
		msgpack_pack_uint16(&pk, tensor->height);
		msgpack_pack_uint16(&pk, tensor->width);
	}

	// Encode tensor data
	f = tensor->data;
	n = tensor->batch * tensor->chan * tensor->height * tensor->width;
	msgpack_pack_array(&pk, n);
	{
		for (i = 0; i < n; i++)
			msgpack_pack_float(&pk, *f++);
	}

	// Encode rescode
	msgpack_pack_int(&pk, rescode);
}

int response_send(int socket, TENSOR * tensor, int rescode)
{
	int ret;
	msgpack_sbuffer sbuf;

	check_tensor(tensor);

	msgpack_sbuffer_init(&sbuf);
	response_encode(tensor, rescode, &sbuf);

	// syslog_info("Respone send ...");
	if ((ret = nn_send(socket, sbuf.data, sbuf.size, 0)) < 0)
	    syslog_error("nn_send: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
	// syslog_info("Respone send OK");

	msgpack_sbuffer_destroy(&sbuf);

	return (ret >= 0) ? RET_OK : RET_ERROR;
}

TENSOR *response_decode(BYTE *buf, int size, int *rescode)
{
	int n;
	WORD dims[4];
	msgpack_unpacked msg;
	msgpack_unpack_return ret;
	size_t off = 0;
	TENSOR *tensor;
	float *f;

	msgpack_unpacked_init(&msg);

	// Decode tensor dims
	ret = msgpack_unpack_next(&msg, (char const *) buf, size, &off);
	if (ret == MSGPACK_UNPACK_SUCCESS) {
		msgpack_object obj = msg.data;
		if (obj.type == MSGPACK_OBJECT_ARRAY && obj.via.array.size != 0) {
			msgpack_object *p = obj.via.array.ptr;
			msgpack_object *const pend = obj.via.array.ptr + obj.via.array.size;
			for (n = 0; p < pend && n < 4; ++p, n++)
				dims[n] = (WORD) (p->via.i64);
		}
	}

	tensor = tensor_create(dims[0], dims[1], dims[2], dims[3]);
	CHECK_TENSOR(tensor);

	// Decode tensor data
	f = tensor->data;
	ret = msgpack_unpack_next(&msg, (char const *) buf, size, &off);
	if (ret == MSGPACK_UNPACK_SUCCESS) {
		msgpack_object obj = msg.data;
		if (obj.type == MSGPACK_OBJECT_ARRAY && obj.via.array.size != 0) {
			msgpack_object *p = obj.via.array.ptr;
			msgpack_object *const pend = obj.via.array.ptr + obj.via.array.size;
			for (; p < pend; ++p)
				*f++ = (float) (p->via.f64);
		}
	}
	// Decode responde code
	*rescode = 0;
	ret = msgpack_unpack_next(&msg, (char const *) buf, size, &off);
	if (ret == MSGPACK_UNPACK_SUCCESS) {
		msgpack_object obj = msg.data;
		if (obj.type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
			*rescode = (int) obj.via.u64;
		} else if (obj.type == MSGPACK_OBJECT_NEGATIVE_INTEGER) {
			*rescode = (int) obj.via.i64;
		}
	}
	// Check buffer decode over status
	if (ret == MSGPACK_UNPACK_PARSE_ERROR) {
		syslog_error("The data in buf is invalid format.");
	}
	msgpack_unpacked_destroy(&msg);

	return tensor;
}


TENSOR *response_recv(int socket, int *rescode)
{
	int size;
	BYTE *buf;
	TENSOR *tensor;

	if ((size = nn_recv(socket, &buf, NN_MSG, 0)) < 0) {
	    syslog_error("nn_recv: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
		return NULL;
	}
	// syslog_info("Response tensor parsing ... ");
	tensor = response_decode(buf, size, rescode);
	// syslog_info("Response tensor parsed OK.");

	nn_freemsg(buf);

	return tensor;
}

void client_close(int socket)
{
	nn_close(socket);
}

void server_close(int socket)
{
	nn_shutdown(socket, 0);
}
