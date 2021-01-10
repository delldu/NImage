/************************************************************************************
***
***	Copyright 2021 Dell(18588220928g@163.com), All Rights Reserved.
***
***	File Author: Dell, 2021-01-09 14:43:18
***
************************************************************************************/

#include "tensor.h"
#include "abhead.h"
#include "image.h"

#define TENSOR_MAGIC MAKE_FOURCC('T', 'E', 'N', 'S')

int tensor_valid(TENSOR * tensor)
{
	return (!tensor || tensor->batch < 1 || tensor->chan < 1 ||
			tensor->height < 1 || tensor->width < 1 || tensor->magic != TENSOR_MAGIC) ? 0 : 1;
}

TENSOR *tensor_create(WORD b, WORD c, WORD h, WORD w)
{
	TENSOR *t;

	BYTE *base = (BYTE *) calloc((size_t) 1, sizeof(TENSOR) + b * c * h * w);
	if (!base) {
		syslog_error("Allocate memeory.");
		return NULL;
	}
	t = (TENSOR *) base;

	t->magic = TENSOR_MAGIC;
	t->batch = b;
	t->chan = c;
	t->height = h;
	t->width = w;
	t->base = base + sizeof(TENSOR);

	return t;
}

TENSOR *tensor_copy(TENSOR *src)
{
	TENSOR *dst;

	CHECK_TENSOR(src);

	dst = tensor_create(src->batch, src->chan, src->height, src->width);
	if (tensor_valid(dst)) {
		memcpy(dst->base, src->base, src->batch * src->chan * src->height * src->width);
	}
	
	return dst;
}

void tensor_destroy(TENSOR * tensor)
{
	if (!tensor_valid(tensor))
		return;

	free(tensor);
}

int tensor_abhead(TENSOR * tensor, BYTE * buffer)
{
	AbHead h;

	check_tensor(tensor);

	abhead_init(&h);
	h.len = tensor->batch * tensor->chan * tensor->height * tensor->width;
	h.b = tensor->batch;
	h.c = tensor->chan;
	h.h = tensor->height;
	h.w = tensor->width;
	h.opc = tensor->opc;

	return abhead_encode(&h, buffer);
}

TENSOR *tensor_fromab(BYTE * buf)
{
	TENSOR *tensor;
	AbHead abhead;

	if (abhead_decode(buf, &abhead) != RET_OK) {
		syslog_error("Bad Ab Head.");
		return NULL;
	}

	tensor = tensor_create(abhead.b, abhead.c, abhead.h, abhead.w);
	CHECK_TENSOR(tensor);
	tensor->opc = abhead.opc;
	memcpy(tensor->base, buf + sizeof(AbHead), abhead.len);

	return tensor;
}

BYTE *tensor_toab(TENSOR * tensor)
{
	BYTE *buf;
	int data_size;

	CHECK_TENSOR(tensor);

	data_size = tensor->batch * tensor->chan * tensor->height * tensor->width;
	buf = (BYTE *) malloc(sizeof(AbHead) + data_size);
	if (!buf) {
		syslog_error("Allocate memory.");
		return NULL;
	}

	tensor_abhead(tensor, buf);
	memcpy(buf + sizeof(AbHead), tensor->base, data_size);

	return buf;
}

#ifdef CONFIG_NNG
int tensor_send(nng_socket socket, TENSOR * tensor)
{
	int ret;
	nng_msg *msg = NULL;
	BYTE head_buf[sizeof(AbHead)];
	size_t send_size;

	check_tensor(tensor);

	tensor_abhead(tensor, head_buf);
	if ((ret = nng_msg_alloc(&msg, 0)) != 0) {
		syslog_error("nng_msg_alloc: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}
	if ((ret = nng_msg_append(msg, head_buf, sizeof(AbHead))) != 0) {
		syslog_error("nng_msg_append: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}
	send_size = tensor->batch * tensor->chan * tensor->height * tensor->width;
	if ((ret = nng_msg_append(msg, tensor->base, send_size)) != 0) {
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

TENSOR *tensor_recv(nng_socket socket)
{
	int ret;
	BYTE *recv_buf = NULL;
	size_t recv_size;
	TENSOR *t = NULL;

	if ((ret = nng_recv(socket, &recv_buf, &recv_size, NNG_FLAG_ALLOC)) != 0) {
		syslog_error("nng_recv: return code = %d, message = %s", ret, nng_strerror(ret));
		nng_free(recv_buf, recv_size);	// Bad message received...
		return NULL;
	}
	if (valid_ab(recv_buf, recv_size))
		t = tensor_fromab(recv_buf);

	nng_free(recv_buf, recv_size);	// Message has been saved ...

	return t;
}

TENSOR *rpc_tensor_tensor(nng_socket socket, TENSOR *src, WORD opc)
{
	CHECK_TENSOR(src);

	if (RPC_IS_TENSOR_TEXT(opc)) {
		syslog_error("Bad RPC opc 0x%x(from tensor to tensor).", opc);
		return NULL;
	}
	src->opc = opc;
	if (tensor_send(socket, src) == RET_OK)
		return tensor_recv(socket);

	syslog_error("RPC from tensor to tensor.");
	return NULL;
}

BYTE *rpc_tensor_text(nng_socket socket, TENSOR *src, WORD opc)
{
	int size;
	CHECK_TENSOR(src);

	if (! RPC_IS_TENSOR_TEXT(opc)) {
		syslog_error("Bad RPC opc 0x%x(from tensor to text).", opc);
		return NULL;
	}
	src->opc = opc;
	if (tensor_send(socket, src) == RET_OK)
		return text_recv(socket, &size);

	syslog_error("RPC from tensor to text.");
	return NULL;
}

#endif
