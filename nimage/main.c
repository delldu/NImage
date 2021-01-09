/************************************************************************************
***
***	Copyright 2020 Dell(18588220928g@163.com), All Rights Reserved.
***
***	File Author: Dell, 2020-11-22 13:18:11
***
************************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <syslog.h>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>

#include <nimage/image.h>

#define URL "ipc:///tmp/nimage.ipc"

#define RPC_MAX_RESPONE_TEXT_LENGTH 1023

// nng/nng.h define ...
// typedef struct nng_socket_s {
//         uint32_t id;
// } nng_socket;

// do rpc from tensor to tensor
TENSOR *rpc_tensor_tensor_image_service(TENSOR *src)
{
	TENSOR *dst = NULL;

	CHECK_TENSOR(src);

	switch(src->opc) {
		case RPC_TENSOR_TENSOR_HELLO:
			dst = tensor_copy(src);
			break;
		case RPC_TENSOR_TENSOR_IMAGE_CLEAN:
			break;
		case RPC_TENSOR_TENSOR_IMAGE_COLOR:
			break;
		case RPC_TENSOR_TENSOR_IMAGE_ZOOM:
			break;
		case RPC_TENSOR_TENSOR_IMAGE_PATH:
			break:
		default:
			syslog_error("NO Implemented service 0x%x.", src->opc);
			break;
	}

	return dst;
}

// do rpc from tensor to text
BYTE *rpc_tensor_text_image_service(TENSOR *src)
{
	BYTE *response = NULL;

	CHECK_TENSOR(src);

	response = (BYTE *)calloc(1, RPC_MAX_RESPONE_TEXT_LENGTH + 1);
	switch(src->opc) {
		case RPC_TENSOR_TEXT_HELLO:
			if (response)
				snprintf((char *)response, RPC_MAX_RESPONE_TEXT_LENGTH, "Hello, Image Service !");
			break;
		default:
			syslog_error("NO Implemented service 0x%x.", src->opc);
			break;
	}

	return response;
}

// start_nimage_server
int start_nimage_server()
{
	int ret;
	nng_socket socket;
	TENSOR *recv_tensor, *send_tensor = NULL;
	BYTE *send_text = NULL;

	int count = 0;

	// sudo journalctl -u image.service -n 10
	syslog(LOG_INFO, "Start image service on %s ...\n", URL);
	if ((ret = nng_rep0_open(&socket)) != 0) {
		syslog_error("nng_rep0_open: : return code = %d, message = %s", ret, nng_strerror(ret));
	}
	if ((ret = nng_listen(socket, URL, NULL, 0)) != 0) {
		syslog_error("nng_listen: : return code = %d, message = %s", ret, nng_strerror(ret));
	}
	syslog(LOG_INFO, "Image service already ...\n");

	for (;;) {
		if (count % 100 == 0) {
			syslog(LOG_INFO, "Do image service %d times\n", count);
		}
		recv_tensor = tensor_recv(socket);

		if (tensor_valid(recv_tensor)) {
			if (RPC_IS_TENSOR_TEXT(recv_tensor->opc)) {
				send_text = rpc_tensor_text_image_service(recv_tensor);
				if (send_text) {
					ret = nng_send(socket, send_text, RPC_MAX_RESPONE_TEXT_LENGTH, NNG_FLAG_NONBLOCK);
					if (ret != 0)
						syslog_error("nng_send: : return code = %d, message = %s", ret, nng_strerror(ret));
					free(send_text);
				}
			} else {
				send_tensor = rpc_tensor_tensor_image_service(recv_tensor);
				if (tensor_valid(send_tensor)) {
					tensor_send(send_tensor);
					tensor_destroy(send_tensor);
				}
			}

			tensor_destroy(recv_tensor);
		}

		count++;
	}

	syslog(LOG_INFO, "Image service shutdown.\n");
	nng_close(socket);

	return RET_OK;
}

IMAGE *call_pix2pix_service(nng_socket socket, IMAGE *send_image)
{
	int ret;
	BYTE *recv_buf = NULL;
	size_t recv_size;
	IMAGE *recv_image = NULL;

	CHECK_IMAGE(send_image);

	// Send ...
	fast_send_image(socket, send_image);

	// Receive ...
	if ((ret = nng_recv(socket, &recv_buf, &recv_size, NNG_FLAG_ALLOC)) != 0) {
		fatal("nng_recv", ret);
	}
	if (valid_ab(recv_buf, recv_size))
		recv_image = image_fromab(recv_buf);

	nng_free(recv_buf, recv_size); // release recv_buf ...

	return recv_image;
}

int client(char *input_file, char *cmd, char *output_file)
{
	int ret;
	nng_socket socket;
	IMAGE *send_image, *recv_image;

	if ((ret = nng_req0_open(&socket)) != 0) {
		fatal("nng_socket", ret);
	}
	if ((ret = nng_dial(socket, URL, NULL, 0)) != 0) {
		fatal("nng_dial", ret);
	}
	send_image = image_load(input_file); check_image(send_image);

	switch(*cmd) {
		case 'c':
			break;
		case 'C':
			break;
		case 'z':
			break;
		case 'p':
			break;
		default:
			break;
	}
	
	// Test performance
	int k;
	time_reset();
	// printf("Test image service performance ...\n");
	for (k = 0; k < 100; k++) {
		recv_image = call_pix2pix_service(socket, send_image);

		if (image_valid(recv_image))
			image_destroy(recv_image);
	}
	time_spend("Image service 100 times");

	recv_image = call_pix2pix_service(socket, send_image);
	if (image_valid(recv_image)) {
		image_save(recv_image, output_file);
		image_destroy(recv_image);
	}

	image_destroy(send_image);

	nng_close(socket);

	return RET_OK;
}

void help(char *cmd)
{
	printf("Usage: %s [option]\n", cmd);
	printf("    -h, --help                   Display this help.\n");
	printf("    -s, --server                 Start server.\n");
	printf("\n");
	printf("    -c, --clean <file>           Clean image.\n");
	printf("    -C, --color <file>           Color image.\n");
	printf("    -z, --zoom <file>            Zoom image.\n");
	printf("    -p, --patch <file>           Patch image.\n");
	printf("\n");
	printf("    -o, --output <file>          Output file (default: output.png).\n");

	exit(1);
}

int main(int argc, char **argv)
{
	int optc;
	int option_index = 0;

	char *clean_file = NULL;
	char *color_file = NULL;
	char *zoom_file = NULL;
	char *patch_file = NULL;
	char *output_file = "output.png";

	struct option long_opts[] = {
		{ "help", 0, 0, 'h'},
		{ "server", 0, 0, 's'},
		{ "clean", 1, 0, 'c'},
		{ "color", 1, 0, 'C'},
		{ "zoom", 1, 0, 'z'},
		{ "patch", 1, 0, 'p'},
		{ "output", 1, 0, 'o'},
		{ 0, 0, 0, 0}
	};

	if (argc <= 1)
		help(argv[0]);
	
	while ((optc = getopt_long(argc, argv, "h s c: C: z: p: o:", long_opts, &option_index)) != EOF) {
		switch (optc) {
		case 's':
			return start_nimage_server();
			break;
		case 'c':
			clean_file = optarg;
			break;
		case 'C':
			color_file = optarg;
			break;
		case 'z':
			zoom_file = optarg;
			break;
		case 'p':
			patch_file = optarg;
			break;
		case 'o':
			output_file = optarg;
			break;
		case 'h':	// help
		default:
			help(argv[0]);
			break;
	    }
	}

	if (clean_file) {
		return client(clean_file, 'c', output_file);
	}
	if (color_file) {
		return client(color_file, 'C', output_file);
	}
	if (zoom_file) {
		return client(zoom_file, 'z', output_file);
	}
	if (patch_file) {
		return client(zoom_file, 'p', output_file);
	}

	help(argv[0]);

	return RET_ERROR;
}
