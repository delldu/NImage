/************************************************************************************
***
***	Copyright 2021 Dell(18588220928g@163.com), All Rights Reserved.
***
***	File Author: Dell, 2021-01-09 14:43:18
***
************************************************************************************/

#include "nngmsg.h"
#include <msgpack.h>

#if 0
// xxxx3333
int text_send(nng_socket socket, BYTE *buf, int size)
{
	int ret;
	AbHead h;
	nng_msg *msg = NULL;
	BYTE head_buf[sizeof(AbHead)];

	abhead_init(&h);
	h.len = size;
	h.b = h.c = h.h = 1;
	h.w = size;
	h.opc = 0;
	abhead_encode(&h, head_buf);

	if ((ret = nng_msg_alloc(&msg, 0)) != 0) {
		syslog_error("nng_msg_alloc: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}
	if ((ret = nng_msg_append(msg, head_buf, sizeof(AbHead))) != 0) {
		syslog_error("nng_msg_append: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}
	if ((ret = nng_msg_append(msg, buf, size)) != 0) {
		syslog_error("nng_msg_append: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}
	if ((ret = nng_sendmsg(socket, msg, NNG_FLAG_ALLOC)) != 0) {
		syslog_error("nng_sendmsg: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}
	// nng_msg_free(msg); // NNG_FLAG_ALLOC means "call nng_msg_free auto"

	return RET_OK;
}

BYTE *text_recv(nng_socket socket, int *size)
{
	int ret;
	BYTE *recv_buf = NULL;
	size_t recv_size;
	BYTE *s = NULL;

	*size = 0;
	if ((ret = nng_recv(socket, &recv_buf, &recv_size, NNG_FLAG_ALLOC)) != 0) {
		syslog_error("nng_recv: return code = %d, message = %s", ret, nng_strerror(ret));
		nng_free(recv_buf, recv_size);	// Bad message received...
		return NULL;
	}

	if (valid_ab(recv_buf, recv_size)) {
		s = (BYTE *)calloc(1, recv_size - sizeof(AbHead) + 1);
		if (s) {
			*size = recv_size - sizeof(AbHead);
			memcpy(s, recv_buf + sizeof(AbHead),  *size);
		}
	}

	nng_free(recv_buf, recv_size);	// Message has been saved ...
	return s;
}
#endif

// typedef struct nng_socket_s {
//         uint32_t id;
// } nng_socket;

int start_server(char *endpoint, nng_socket *socket)
{
	int ret;

	// sudo journalctl -u image.service -n 10
	syslog_info("Start service on %s ...\n", endpoint);
	if ((ret = nng_rep_open(socket)) != 0) {
		syslog_error("nng_rep_open: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}

	if ((ret = nng_listen(*socket, endpoint, NULL, 0)) != 0) {
		syslog_error("nng_listen: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}

	syslog_info("Service already ...\n");

	return RET_OK;
}

int client_connect(char *endpoint, nng_socket *socket)
{
	int ret;

	if ((ret = nng_req_open(socket)) != 0) {
		syslog_error("nng_socket: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}
    // ret = nng_socket_set_ms(socket, NNG_OPT_RECVTIMEO, 5000);
    // if (ret != 0) {
    //         fatal("nng_socket_set(nng_opt_recvtimeo)", ret);
    // }
	if ((ret = nng_dial(*socket, endpoint, NULL, 0)) != 0) {
		syslog_error("nng_dial: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}

	return RET_OK;
}


/****************************************************************************
* Request Tensor format:
*	int reqcode,
*	Tensor: uint16 [BxCxHxW], float [d1, ..., dn]
*	float option
****************************************************************************/
int request_send(nng_socket socket, int reqcode, TENSOR *tensor, float option)
{
	int ret;
	size_t i, n;
	float *f;
	msgpack_sbuffer sbuf;
    msgpack_packer pk;

    check_tensor(tensor);

    msgpack_sbuffer_init(&sbuf);
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    // Encode reqcode
    msgpack_pack_int(&pk, reqcode);

    // Encode tensor dims
    msgpack_pack_array(&pk, 4);
    msgpack_pack_uint16(&pk, tensor->batch);
    msgpack_pack_uint16(&pk, tensor->chan);
    msgpack_pack_uint16(&pk, tensor->height);
    msgpack_pack_uint16(&pk, tensor->width);

    // Encode tensor data
    f = tensor->data;
    n = tensor->batch * tensor->chan * tensor->height * tensor->width;
    msgpack_pack_array(&pk, n);
    for (i = 0; i < n; i++)
    	msgpack_pack_float(&pk, *f++);

    // Encode option
    msgpack_pack_float(&pk, option);

	if ((ret = nng_send(socket, sbuf.data, sbuf.size, 0)) != 0)
		syslog_error("request_send: return code = %d, message = %s", ret, nng_strerror(ret));

    msgpack_sbuffer_destroy(&sbuf);

    return (ret == 0)?RET_OK : RET_ERROR;
}

TENSOR *request_recv(nng_socket socket, int *reqcode, float *option)
{
	int n, ret;
	WORD dims[4];
	BYTE *buf = NULL;
	size_t size;
    msgpack_unpacked result;
    msgpack_unpack_return msgret;
    size_t off = 0;
    TENSOR *tensor;
    float *f;

	if ((ret = nng_recv(socket, &buf, &size, NNG_FLAG_ALLOC)) != 0) {
		syslog_error("request_recv: return code = %d, message = %s", ret, nng_strerror(ret));
		nng_free(buf, size);	// Bad message received...
		return NULL;
	}

    msgpack_unpacked_init(&result);

    // Decode reqcode
    *reqcode = 0;
    msgret = msgpack_unpack_next(&result, (char const*)buf, size, &off);
    if (msgret == MSGPACK_UNPACK_SUCCESS) {
        msgpack_object obj = result.data;
        if (obj.type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
        	*reqcode = (int)obj.via.u64;
        } else if (obj.type == MSGPACK_OBJECT_NEGATIVE_INTEGER) {
        	*reqcode = (int)obj.via.i64;
        }
    }

    // Decode tensor dims
    msgret = msgpack_unpack_next(&result, (char const*)buf, size, &off);
    if (msgret == MSGPACK_UNPACK_SUCCESS) {
        msgpack_object obj = result.data;
        if (obj.type == MSGPACK_OBJECT_ARRAY && obj.via.array.size != 0 ) {
	        msgpack_object* p = obj.via.array.ptr;
	        msgpack_object* const pend = obj.via.array.ptr + obj.via.array.size;
	        for(n = 0; p < pend && n < 4; ++p, n++) {
	        	dims[n] = (WORD)(p->via.i64);
	        }
        }
    }

    tensor = tensor_create(dims[0], dims[1], dims[2], dims[3]); CHECK_TENSOR(tensor);

    // Decode tensor data
    f = tensor->data;
    msgret = msgpack_unpack_next(&result, (char const*)buf, size, &off);
    if (msgret == MSGPACK_UNPACK_SUCCESS) {
        msgpack_object obj = result.data;
        if (obj.type == MSGPACK_OBJECT_ARRAY && obj.via.array.size != 0 ) {
	        msgpack_object* p = obj.via.array.ptr;
	        msgpack_object* const pend = obj.via.array.ptr + obj.via.array.size;
	        for(; p < pend; ++p)
	        	*f++ = (float)(p->via.f64);
        }
    }

    // Decode tensor option
    *option = 0;
    msgret = msgpack_unpack_next(&result, (char const*)buf, size, &off);
    if (msgret == MSGPACK_UNPACK_SUCCESS) {
        msgpack_object obj = result.data;
        if (obj.type == MSGPACK_OBJECT_FLOAT) {
        	*option = (float)obj.via.f64;
        }
    }

    // Check buffer decode over status
	if (msgret == MSGPACK_UNPACK_PARSE_ERROR) {
        fprintf(stderr, "The data in buf is invalid format.\n");
	}
    msgpack_unpacked_destroy(&result);

	nng_free(buf, size);	// Message has been saved ...

    return tensor;
}

/****************************************************************************
* Response Tensor format:
*	Tensor: uint16 [BxCxHxW], float [d1, ..., dn]
*	int rescode
****************************************************************************/
int response_send(nng_socket socket, TENSOR *tensor, int rescode)
{
	int ret;
	size_t i, n;
	float *f;
	msgpack_sbuffer sbuf;
    msgpack_packer pk;

    check_tensor(tensor);

    msgpack_sbuffer_init(&sbuf);
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    // Encode tensor dims
    msgpack_pack_array(&pk, 4);
    msgpack_pack_uint16(&pk, tensor->batch);
    msgpack_pack_uint16(&pk, tensor->chan);
    msgpack_pack_uint16(&pk, tensor->height);
    msgpack_pack_uint16(&pk, tensor->width);

    // Encode tensor data
    f = tensor->data;
    n = tensor->batch * tensor->chan * tensor->height * tensor->width;
    msgpack_pack_array(&pk, n);
    for (i = 0; i < n; i++)
    	msgpack_pack_float(&pk, *f++);

    // Encode rescode
    msgpack_pack_int(&pk, rescode);

	if ((ret = nng_send(socket, sbuf.data, sbuf.size, 0)) != 0)
		syslog_error("response_send: return code = %d, message = %s", ret, nng_strerror(ret));

    msgpack_sbuffer_destroy(&sbuf);

	return (ret == 0)?RET_OK : RET_ERROR;
}

TENSOR *response_recv(nng_socket socket, int *rescode)
{
	int n, ret;
	WORD dims[4];
	BYTE *buf = NULL;
	size_t size;
    msgpack_unpacked result;
    msgpack_unpack_return msgret;
    size_t off = 0;
    TENSOR *tensor;
    float *f;

	if ((ret = nng_recv(socket, &buf, &size, NNG_FLAG_ALLOC)) != 0) {
		syslog_error("request_recv: return code = %d, message = %s", ret, nng_strerror(ret));
		nng_free(buf, size);	// Bad message received...
		return NULL;
	}

    msgpack_unpacked_init(&result);

    // Decode tensor dims
    msgret = msgpack_unpack_next(&result, (char const*)buf, size, &off);
    if (msgret == MSGPACK_UNPACK_SUCCESS) {
        msgpack_object obj = result.data;
        if (obj.type == MSGPACK_OBJECT_ARRAY && obj.via.array.size != 0 ) {
	        msgpack_object* p = obj.via.array.ptr;
	        msgpack_object* const pend = obj.via.array.ptr + obj.via.array.size;
	        for(n = 0; p < pend && n < 4; ++p, n++) {
	        	dims[n] = (WORD)(p->via.i64);
	        }
        }
    }

    tensor = tensor_create(dims[0], dims[1], dims[2], dims[3]); CHECK_TENSOR(tensor);

    // Decode tensor data
    f = tensor->data;
    msgret = msgpack_unpack_next(&result, (char const*)buf, size, &off);
    if (msgret == MSGPACK_UNPACK_SUCCESS) {
        msgpack_object obj = result.data;
        if (obj.type == MSGPACK_OBJECT_ARRAY && obj.via.array.size != 0 ) {
	        msgpack_object* p = obj.via.array.ptr;
	        msgpack_object* const pend = obj.via.array.ptr + obj.via.array.size;
	        for(; p < pend; ++p)
	        	*f++ = (float)(p->via.f64);
        }
    }

    // Decode responde code
    *rescode = 0;
    msgret = msgpack_unpack_next(&result, (char const*)buf, size, &off);
    if (msgret == MSGPACK_UNPACK_SUCCESS) {
        msgpack_object obj = result.data;
        if (obj.type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
        	*rescode = (int)obj.via.u64;
        } else if (obj.type == MSGPACK_OBJECT_NEGATIVE_INTEGER) {
        	*rescode = (int)obj.via.i64;
        }
    }

    // Check buffer decode over status
	if (msgret == MSGPACK_UNPACK_PARSE_ERROR) {
        fprintf(stderr, "The data in buf is invalid format.\n");
	}
    msgpack_unpacked_destroy(&result);

	nng_free(buf, size);	// Message has been saved ...

    return tensor;
}

