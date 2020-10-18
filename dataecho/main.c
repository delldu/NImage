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
 #include <syslog.h> // syslog, RFC3164 ?


#define WORD_LOW(w) ((BYTE)(w & 0xff))
#define WORD_HI(w) ((BYTE)((w & 0xff00) >> 8))
#define WORD_FROM_BYTES(low, hi) (((hi & 0xff) << 8) | low)

typedef struct {
	BYTE t[2];	// 2 bytes
	DWORD len;	// 4 bytes
	WORD crc;	// 2 bytes
} AbHead;

// ImageData
typedef struct {
	WORD h, w, c, opc;
} ImageDataHead;

extern int image_data_head_decode(BYTE *buf, ImageDataHead *head);
extern BYTE *image_data_encode(IMAGE *image, int opcode);
extern IMAGE *image_data_decode(ImageDataHead *head, BYTE *body);

#define CRC_CCITT_POLY 0x1021 	//CRC-CCITT, polynormial 0x1021.
// 0x31 0x32 0x33 0x34 0x35 0x36 ==> 0x20E4
// ab -- ArrayBuffer, AbHead == {'ab', 4 bytes len, crc16}, total 8 bytes
WORD ab_head_crc(BYTE *buf, int length)
{
    int i;
    WORD crc;
  
    crc = 0;
    while(--length >= 0) {
        crc = crc ^ ((WORD) (*buf++ << 8));
        for(i = 0; i < 8; i++) {
            if( crc & 0x8000 )
                crc = (crc << 1) ^ CRC_CCITT_POLY;
            else
                crc = crc << 1;
        }
    }

    return crc;
}

int ab_head_decode(BYTE *buf, AbHead *head)
{
	// t[2], len-32, crc --16
	head->t[0] = buf[0];
	head->t[1] = buf[1];
	head->len = MAKE_FOURCC(buf[2],buf[3],buf[4],buf[5]);
	head->crc = WORD_FROM_BYTES(buf[6], buf[7]);

	return (head->len > 0 && head->crc == ab_head_crc(buf, 6))?RET_OK : RET_ERROR;
}

int ab_head_encode(AbHead *head, BYTE *buf)
{
	buf[0] = head->t[0];
	buf[1] = head->t[1];
	buf[2] = GET_FOURCC1(head->len);
	buf[3] = GET_FOURCC2(head->len);
	buf[4] = GET_FOURCC3(head->len);
	buf[5] = GET_FOURCC4(head->len);

	head->crc = ab_head_crc(buf, 6);
	buf[6] = WORD_LOW(head->crc);
	buf[7] = WORD_HI(head->crc);

	return RET_OK;
}

int image_data_head_decode(BYTE *buf, ImageDataHead *head)
{
	// h, w, c, opc
	head->h = WORD_FROM_BYTES(buf[0], buf[1]);
	head->w = WORD_FROM_BYTES(buf[2], buf[3]);
	head->c = WORD_FROM_BYTES(buf[4], buf[5]);
	head->opc = WORD_FROM_BYTES(buf[6], buf[7]);

	return (head->h > 0 && head->w > 0 && head->c > 0)?RET_OK : RET_ERROR;
}

int image_data_head_encode(ImageDataHead *head, BYTE *buf)
{
	// h, w, c, opc
	buf[0] = WORD_LOW(head->h);
	buf[1] = WORD_HI(head->h);
	buf[2] = WORD_LOW(head->w);
	buf[3] = WORD_HI(head->w);
	buf[4] = WORD_LOW(head->c);
	buf[5] = WORD_HI(head->c);
	buf[6] = WORD_LOW(head->opc);
	buf[7] = WORD_HI(head->opc);

	return RET_OK;
}

BYTE *image_data_encode(IMAGE *image, int opcode)
{
	ssize_t length;
	AbHead abhead;
	ImageDataHead head;

	BYTE *arraybuffer;

	CHECK_IMAGE(image);
	length = 4 * image->height * image->width;
	arraybuffer = (BYTE *)calloc((size_t)1, 8 + 8 + length);
	if (! arraybuffer) {
		syslog(LOG_DEBUG, "Allocate memory for image_data_encode.");
		return NULL;
	}

	// Set arraybuffer;
	// 1. encode abhead
	abhead.t[0] = 'a';
	abhead.t[1] = 'b';
	abhead.len = 8 + length;
	ab_head_encode(&abhead, arraybuffer);

	// 2. encode image head
	head.h = image->height;
	head.w = image->width;
	head.c = 4;
	head.opc = opcode;
	image_data_head_encode(&head, arraybuffer + 8);

	// 3. encode image data
	memcpy(arraybuffer + 8 + 8, image->base, length);

	return arraybuffer;
}

IMAGE *image_data_decode(ImageDataHead *head, BYTE *body)
{
	IMAGE *image;

	image = image_create(head->h, head->w); CHECK_IMAGE(image);

	memcpy(image->base, body, 4 * image->height * image->width);

	return image;
}

void dataecho_help(char *cmd)
{
	printf("Usage: %s [option]\n", cmd);
	printf("    -h, --help                   Display this help.\n");

	exit(1);
}

IMAGE* get_request_image()
{
	BYTE headbuf[9], *databuf;
	ssize_t n, length;
	AbHead abhead;
	ImageDataHead head;
	IMAGE *image;

	if (read(0, headbuf, 8) != 8) {
		syslog(LOG_DEBUG, "Reading array buffer head error.\n");
		return NULL;
	}
	// 1. Get abhead ?
	if (ab_head_decode(headbuf, &abhead) != RET_OK) {
		syslog(LOG_DEBUG, "Bad AbHead: t = %c%c, len = %d, crc = %x .\n",
			abhead.t[0], abhead.t[1], abhead.len, abhead.crc);
		syslog(LOG_DEBUG, "AbHead detail informaton: %d,%d,%d,%d,%d,%d,%d,%d",
			headbuf[0], headbuf[1], headbuf[2], headbuf[3], headbuf[4], headbuf[5], headbuf[6], headbuf[7]);
		syslog(LOG_DEBUG, "\n");
		while (read(0, headbuf, 8) > 0)
			; // Skip left dirty data ...

		return NULL;
	}

	// 2. Get image head ?
	if (read(0, headbuf, 8) != 8) {
		syslog(LOG_DEBUG, "Reading image head error.\n");
		return NULL;
	}
	if (image_data_head_decode(headbuf, &head) != RET_OK) {
		syslog(LOG_DEBUG, "Bad ImageHead.\n");
		syslog(LOG_DEBUG, "ImageHead detail informatoin: HxWxC = %dx%dx%d, opc = %d.\n",
			 head.h, head.w, head.c, head.opc);
		while (read(0, headbuf, 8) > 0)
			; // Skip left dirty data ...
		return NULL;
	}
	syslog(LOG_DEBUG, "Good ImageHead: HxWxC = %dx%dx%d, opc = %d.\n",
		 head.h, head.w, head.c, head.opc);

	// 3. Get image data
	image = image_create(head.h, head.w); CHECK_IMAGE(image);
	image->opc = head.opc;	// Save RPC Method

	length = 4 * head.h * head.w * sizeof(BYTE);
	databuf = (BYTE *)image->base;
	n = 0;
	while (n < length) {
		n += read(0, databuf + n, length - n);
		if (n <= 0)
			break;
	}
	if (n != length) {
		syslog(LOG_DEBUG, "Error: Received %ld bytes image data -- expect %ld !!!\n", n, length);
		image_destroy(image);
		return NULL;
	}

	return image;
}

int out_response_image(IMAGE *image)
{
	size_t length;
	check_image(image);

	BYTE *response = image_data_encode(image, image->opc);
	if (response) {
		length = 4 * image->height * image->width;
		length = write(1, response, 8 + 8 + length);
		free(response);
		return RET_OK;
	}

	return RET_ERROR;
}

int image_service()
{
	IMAGE *image;

	image = get_request_image();
	check_image(image);
	// Process image ...
	// switch(request_image->opc) {
	// 	case 1:
	// 		break;
	// 	case 2:
	// 		break;
	// 	default:
	// 		break;
	// }
	out_response_image(image);

	image_destroy(image);
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

	// cat /var/log/syslog | grep -i dataecho ...
	syslog(LOG_DEBUG, "Start image service now ... \n");
	while(1)
		image_service();

	return 0;
}

