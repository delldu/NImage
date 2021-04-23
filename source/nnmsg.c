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

/****************************************************************************
* Format:
*	int msgcode,
*	Tensor: uint16 [BxCxHxW], float [d1, ..., dn]
*	float option
****************************************************************************/
void tensor_encode(int msgcode, TENSOR * tensor, msgpack_sbuffer *sbuf)
{
	int i, n;
	float *f;
	msgpack_packer pk;

	msgpack_packer_init(&pk, sbuf, msgpack_sbuffer_write);

	// Encode msgcode
	msgpack_pack_int(&pk, msgcode);

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
}

TENSOR *tensor_decode(BYTE *buf, int size, int *msgcode)
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
	*msgcode = 0;
	ret = msgpack_unpack_next(&msg, (char const *) buf, size, &off);
	if (ret == MSGPACK_UNPACK_SUCCESS) {
		msgpack_object obj = msg.data;
		if (obj.type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
			*msgcode = (int) obj.via.u64;
		} else if (obj.type == MSGPACK_OBJECT_NEGATIVE_INTEGER) {
			*msgcode = (int) obj.via.i64;
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
	// Check buffer decode over status
	if (ret == MSGPACK_UNPACK_PARSE_ERROR) {
		syslog_error("The data in msgpack buffer is invalid format.");
	}
	msgpack_unpacked_destroy(&msg);

	return tensor;
}

int tensor_send(int socket, int msgcode, TENSOR * tensor)
{
	int ret;
	msgpack_sbuffer sbuf;

	check_tensor(tensor);

	msgpack_sbuffer_init(&sbuf);
	tensor_encode(msgcode, tensor, &sbuf);

	// syslog_info("Tensor send ... size = %d", sbuf.size);
	if ((ret = nn_send(socket, sbuf.data, sbuf.size, 0)) < 0)
	    syslog_error("nn_send: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
	// syslog_info("Tensor send OK");

	msgpack_sbuffer_destroy(&sbuf);

	return (ret >= 0) ? RET_OK : RET_ERROR;
}

TENSOR *tensor_recv(int socket, int *msgcode)
{
	return tensor_recv_timeout(socket, 0, msgcode);
}

TENSOR *tensor_recv_timeout(int socket, int timeout, int *msgcode)
{
	int size;
	BYTE *buf;
	TENSOR *tensor;
	struct nn_pollfd pfd[1];
	int rc = 0;

	if (timeout > 0) {
		pfd[0].fd = socket;
		pfd[0].events = NN_POLLIN | NN_POLLOUT;
		rc = nn_poll(pfd, ARRAY_SIZE(pfd), timeout);

		if (rc == 0) {
		    syslog_error("nn_poll: Timeout");
		    return NULL;
		}
		if (rc == -1) {
		    syslog_error("nn_poll: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
		    return NULL;
		}
		if (pfd [0].revents & NN_POLLIN) {
		    syslog_info("Message can be received from socket");
		}
	}

	if ((size = nn_recv(socket, &buf, NN_MSG, 0)) < 0) {
	    syslog_error("nn_recv: error code = %d, message = %s", nn_errno(), nn_strerror(nn_errno()));
		return NULL;
	}

	syslog_info("Tensor parsing ... ");
	tensor = tensor_decode(buf, size, msgcode);
	syslog_info("Tensor parsed OK.");

	nn_freemsg(buf);		// Message has been saved ...

	return tensor;
}

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
	int recv_timeout = 600 * 1000;		// 60 seconds

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

void client_close(int socket)
{
	nn_close(socket);
}

void server_close(int socket)
{
	nn_shutdown(socket, 0);
}

TENSOR *service_request(int socket, int expected_msgcode)
{
	TENSOR *tensor;
	int recv_msgcode;

	tensor = tensor_recv(socket, &recv_msgcode);
    if (recv_msgcode == HELLO_REQUEST_MESSAGE) {
        tensor_send(socket, HELLO_RESPONSE_MESSAGE, tensor);
        tensor_destroy(tensor);
        return NULL;
    }
	if (! tensor_valid(tensor)) {
		syslog_error("Receive error tensor or timeout");
		// Create a fake tensor to client ...
		tensor = tensor_create(1, 1, 1, 1); CHECK_TENSOR(tensor);
		tensor_send(socket, ERROR_TIMEOUT_MESSAGE, tensor);
		tensor_destroy(tensor);
		return NULL;
	}
	// NOT our service ...
	if (recv_msgcode != expected_msgcode) {
		syslog_error("Message 0x%x is not for our service (0x%x)", recv_msgcode, expected_msgcode);
		tensor_send(socket, OUT_OF_SERVICE, tensor);
		tensor_destroy(tensor);
		return NULL;
	}
	return tensor;
}

int service_response(int socket, int msgcode, TENSOR *tensor)
{
	if (tensor_valid(tensor))
		return tensor_send(socket, msgcode, tensor);

	// If service failure, echo response !!!
	tensor = tensor_create(1, 1, 1, 1); check_tensor(tensor);
	tensor_send(socket, INVALID_SERVICE_MESSAGE, tensor);
	tensor_destroy(tensor);

	return RET_ERROR;
}

int service_avaible(int socket)
{
	int recv_msgcode;
	TENSOR *send, *recv;

	send = tensor_create(1, 1, 1, 1); check_tensor(send);
	tensor_send(socket, HELLO_REQUEST_MESSAGE, send);
	tensor_destroy(send);

	recv = tensor_recv_timeout(socket, 2000, &recv_msgcode); // 2000 ms
	check_tensor(recv);
	tensor_destroy(recv);

	return (recv_msgcode == HELLO_RESPONSE_MESSAGE)? RET_OK : RET_ERROR;
}
