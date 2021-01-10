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

#define RPC_MAX_TEXT_LENGTH 1024

// nng/nng.h define ...
// typedef struct nng_socket_s {
//         uint32_t id;
// } nng_socket;

// Run image service(from tensor to tensor)
TENSOR *tensor_tensor_service(TENSOR *src)
{
	TENSOR *dst = NULL;

	CHECK_TENSOR(src);

	switch(src->opc) {
		case RPC_TENSOR_TENSOR_IMAGE_CLEAN:
			dst = tensor_copy(src);
			break;
		case RPC_TENSOR_TENSOR_IMAGE_COLOR:
			dst = tensor_copy(src);
			break;
		case RPC_TENSOR_TENSOR_IMAGE_ZOOM:
			dst = tensor_copy(src);
			break;
		case RPC_TENSOR_TENSOR_IMAGE_PATCH:
			dst = tensor_copy(src);
			break;
		default:
			syslog_error("Bad service opc 0x%x.", src->opc);
			break;
	}

	return dst;
}

// Run image service(from tensor to text)
BYTE *tensor_text_service(TENSOR *src)
{
	BYTE *response = NULL;

	CHECK_TENSOR(src);

	response = (BYTE *)calloc(1, RPC_MAX_TEXT_LENGTH);
	if (! response) {
		syslog_error("Memory allocate.");
		return NULL;
	}
	switch(src->opc) {
		case RPC_TENSOR_TEXT_IMAGE_NIMA:
			snprintf((char *)response, RPC_MAX_TEXT_LENGTH - 1, "Image NIMA 5.50");
			break;
		default:
			syslog_error("Bad service opc 0x%x.", src->opc);
			break;
	}

	return response;
}

// start server
int server()
{
	int ret;
	nng_socket socket;
	TENSOR *r_tensor, *s_tensor = NULL;
	BYTE *s_text = NULL;

	int count = 0;

	// sudo journalctl -u image.service -n 10
	syslog_info("Start image service on %s ...\n", URL);
	if ((ret = nng_rep0_open(&socket)) != 0) {
		syslog_error("nng_rep0_open: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}

	if ((ret = nng_listen(socket, URL, NULL, 0)) != 0) {
		syslog_error("nng_listen: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}

	syslog_info("Image service already ...\n");

	for (;;) {
		if (count % 100 == 0)
			syslog_info("Image service %d times", count);

		r_tensor = tensor_recv(socket);
		if (! tensor_valid(r_tensor))
			continue;

		if (RPC_IS_TENSOR_TEXT(r_tensor->opc)) {
			s_text = tensor_text_service(r_tensor);
			if (s_text) {
				text_send(socket, s_text, RPC_MAX_TEXT_LENGTH);
				free(s_text);
			}
		} else {
			s_tensor = tensor_tensor_service(r_tensor);
			if (tensor_valid(s_tensor)) {
				tensor_send(socket, s_tensor);
				tensor_destroy(s_tensor);
			}
		}

		tensor_destroy(r_tensor);

		count++;
	}

	syslog(LOG_INFO, "Image service shutdown.\n");
	nng_close(socket);

	return RET_OK;
}

int client(char *input_file, WORD opc, char *output_file)
{
	int ret;
	nng_socket socket;
	IMAGE *s_image, *r_image;
	TENSOR *s_tensor, *r_tensor;
	BYTE *r_text = NULL;

	if ((ret = nng_req0_open(&socket)) != 0) {
		syslog_error("nng_socket: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}
	if ((ret = nng_dial(socket, URL, NULL, 0)) != 0) {
		syslog_error("nng_dial: return code = %d, message = %s", ret, nng_strerror(ret));
		return RET_ERROR;
	}

	ret = RET_ERROR;
	s_image = image_load(input_file);
	if (! image_valid(s_image))
		goto finish;

	s_tensor = tensor_from_image(s_image);
	if (RPC_IS_TENSOR_TEXT(opc)) {
		r_text = rpc_tensor_text(socket, s_tensor, opc);
		if (r_text) {
			printf("%s\n", r_text);
			free(r_text);
			ret = RET_OK;
		}
		tensor_destroy(s_tensor);
		image_destroy(s_image);

		goto finish;
	}

	// Now RPC IS from tensor to tensor
	if (tensor_valid(s_tensor)) {
		r_tensor = rpc_tensor_tensor(socket, s_tensor, opc);
		if (tensor_valid(r_tensor)) {

			// Process recv tensor ...
			r_image = image_from_tensor(r_tensor, 0);
			if (image_valid(r_image)) {
				ret = image_save(r_image, output_file);
				image_destroy(r_image);
			}

			tensor_destroy(r_tensor);
		}
		tensor_destroy(s_tensor);			
	}
	image_destroy(s_image);


finish:
	nng_close(socket);

	return ret;
}

void help(char *cmd)
{
	printf("Usage: %s [option]\n", cmd);
	printf("    h, --help                   Display this help.\n");
	printf("    s, --server                 Start server.\n");
	printf("       --clean  <file>          Clean image.\n");
	printf("       --color  <file>          Color image.\n");
	printf("       --zoom   <file>          Zoom image.\n");
	printf("       --patch  <file>          Patch image.\n");
	printf("       --nima   <file>          NIMA image.\n");
	printf("    o, --output <file>          Output file (default: output.png).\n");

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
	char *nima_file = NULL;
	char *output_file = (char *)"output.png";

	struct option long_opts[] = {
		{ "help", 0, 0, 'h'},
		{ "server", 0, 0, 's'},
		{ "clean", 1, 0, 'c'},
		{ "color", 1, 0, 'C'},
		{ "zoom", 1, 0, 'z'},
		{ "patch", 1, 0, 'p'},
		{ "nima", 1, 0, 'n'},
		{ "output", 1, 0, 'o'},
		{ 0, 0, 0, 0}
	};

	if (argc <= 1)
		help(argv[0]);
	
	while ((optc = getopt_long(argc, argv, "h s c: C: z: p: n: o:", long_opts, &option_index)) != EOF) {
		switch (optc) {
		case 's':
			return server();
			break;
		case 'c':	// Clean
			clean_file = optarg;
			break;
		case 'C':	// Color
			color_file = optarg;
			break;
		case 'z':	// Zoom
			zoom_file = optarg;
			break;
		case 'p':	// Patch
			patch_file = optarg;
			break;
		case 'n':	// Nima
			nima_file = optarg;
			break;
		case 'o':	// Output
			output_file = optarg;
			break;
		case 'h':	// help
		default:
			help(argv[0]);
			break;
	    }
	}

	if (clean_file) {
		return client(clean_file, RPC_TENSOR_TENSOR_IMAGE_CLEAN, output_file);
	}
	if (color_file) {
		return client(color_file, RPC_TENSOR_TENSOR_IMAGE_COLOR, output_file);
	}
	if (zoom_file) {
		return client(zoom_file, RPC_TENSOR_TENSOR_IMAGE_ZOOM, output_file);
	}
	if (patch_file) {
		return client(zoom_file, RPC_TENSOR_TENSOR_IMAGE_PATCH, output_file);
	}
	if (nima_file) {
		return client(nima_file, RPC_TENSOR_TEXT_IMAGE_NIMA, output_file);
	}

	help(argv[0]);

	return RET_ERROR;
}
