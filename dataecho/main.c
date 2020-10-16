/************************************************************************************
#***
#***	File Author: Dell, Fri May 30 12:46:39 CST 2020
#***
#************************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <image.h>

void dataecho_help(char *cmd)
{
	printf("Usage: %s [option]\n", cmd);
	printf("    -h, --help                   Display this help.\n");

	exit(1);
}

int echoService()
{
	int i;
	BYTE headbuf[9];
	BYTE *databuf;
	ssize_t n, length;
	FILE *log;

	ImageDataHead head;

	if ((log = fopen("/tmp/websocket.log", "a+")) == NULL) {
		fprintf(stderr, "Create log error ...\n");
		return RET_ERROR;
	}

	fprintf(log, "Start image echo service 02...\n");
	
	if (read(0, headbuf, 8) != 8) {
		fprintf(log, "Reading image data head error.\n");
		fclose(log);

		return RET_ERROR;
	} else {
		headbuf[8] = '\0';
		fprintf(log, "Head: %s\n", headbuf);
	}

	if (image_data_head_decode(headbuf, &head) != RET_OK) {
		fprintf(log, "Bad image data head: h = %d, w = %d, opc = %d, crc = %x .\n",
			head.h, head.w, head.opc, head.crc);
		for (i = 0; i < 8; i++) {
			fprintf(log, "detail data ...... i = %d -- %d\n", i, headbuf[i]);
		}
		fclose(log);

		while (read(0, headbuf, 8) > 0)
			; // Skip left dirty data ...
		return RET_ERROR;
	}
	fprintf(log, "Received HxW = %dx%d, opc = %d, CRC = 0x%0X .\n",
		 head.h, head.w, head.opc, head.crc);

	length = 4 * head.h * head.w * sizeof(BYTE);
	databuf = calloc((size_t)1, length);
	if (! databuf) {
		fprintf(log, "Allocate memory.\n");
		fclose(log);
		return RET_ERROR;
	}

	n = 0;
	while (n < length) {
		n += read(0, databuf + n, length - n);
		fprintf(log, "Reading image data body %ld.\n", n);
		if (n <= 0)
			break;
	}
	if (n != length) {
		fprintf(log, "Received %ld bytes data(body) -- bad !!!\n", n);
		fclose(log);
		free(databuf);
		return RET_ERROR;
	}

	IMAGE *image = image_data_decode(&head, databuf);
	check_image(image);
	// Process image with head->opc ...
	// switch(head.opc) {
	// 	case 1:
	// 		break;
	// 	case 2:
	// 		break;
	// 	default:
	// 		break;
	// }
	BYTE *response = image_data_encode(image, head.opc);
	for (i = 0; i < 8; i++)
		fprintf(log, "write: %d, %d\n", i, response[i]);
	length = 4 * image->height * image->width;
	length = write(1, response, 8 + length);
	free(response);
	image_destroy(image);

	free(databuf);

	fclose(log);

	return RET_OK;
}

int main(int argc, char **argv)
{
	int optc;
	int option_index = 0;

	struct option long_opts[] = {
		{ "help", 0, 0, 'h'},
		{ 0, 0, 0, 0}

	};

	while ((optc = getopt_long(argc, argv, "h", long_opts, &option_index)) != EOF) {
		switch (optc) {
		case 'h':	// help
		default:
			dataecho_help(argv[0]);
			break;
    	}
	}

	while(1)
		echoService();

	// MS -- Modify Section ?

	return 0;
}

