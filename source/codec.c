/************************************************************************************
#***
#***	File Author: Dell, Fri May 30 12:46:39 CST 2020
#***
#************************************************************************************/

#include "image.h"

// Big Endian Order
#define WORD_LOW(w) ((BYTE)(w & 0xff))
#define WORD_HI(w) ((BYTE)((w & 0xff00) >> 8))
#define WORD_FROM_BYTES(low, hi) (((hi & 0xff) << 8) | low)

typedef struct {
	BYTE t[2];	// 2 bytes
	DWORD len;	// 4 bytes
	WORD crc;	// 2 bytes
} AbHead;	// ArrayBuffer Head

typedef struct {
	WORD h, w, c, opc;
} ImgHead;	// Image Head

#define CRC_CCITT_POLY 0x1021 	//CRC-CCITT, polynormial 0x1021.
// 0x31 0x32 0x33 0x34 0x35 0x36 ==> 0x20E4
// ab -- ArrayBuffer, AbHead == {'ab', 4 bytes len, crc16}, total 8 bytes
WORD _abhead_crc16(BYTE *buf, int image_data_size)
{
    int i;
    WORD crc;
  
    crc = 0;
    while(--image_data_size >= 0) {
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

int _abhead_decode(BYTE *buf, AbHead *head)
{
	// t[2], len-32, crc --16
	head->t[0] = buf[0];
	head->t[1] = buf[1];
	head->len = MAKE_FOURCC(buf[2],buf[3],buf[4],buf[5]);
	head->crc = WORD_FROM_BYTES(buf[6], buf[7]);

	return (head->len > 0 && head->crc == _abhead_crc16(buf, 6))?RET_OK : RET_ERROR;
}

int _abhead_encode(AbHead *head, BYTE *buf)
{
	buf[0] = head->t[0];
	buf[1] = head->t[1];
	buf[2] = GET_FOURCC1(head->len);
	buf[3] = GET_FOURCC2(head->len);
	buf[4] = GET_FOURCC3(head->len);
	buf[5] = GET_FOURCC4(head->len);

	head->crc = _abhead_crc16(buf, 6);
	buf[6] = WORD_LOW(head->crc);
	buf[7] = WORD_HI(head->crc);

	return RET_OK;
}

int _imghead_decode(BYTE *buf, ImgHead *head)
{
	// h, w, c, opc
	head->h = WORD_FROM_BYTES(buf[0], buf[1]);
	head->w = WORD_FROM_BYTES(buf[2], buf[3]);
	head->c = WORD_FROM_BYTES(buf[4], buf[5]);
	head->opc = WORD_FROM_BYTES(buf[6], buf[7]);

	return (head->h > 0 && head->w > 0 && head->c > 0)?RET_OK : RET_ERROR;
}

int _imghead_encode(ImgHead *head, BYTE *buf)
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


BYTE *image_encode(IMAGE *image)
{
	// Buffer == AbHead + ImgHead + Image Data
	AbHead abhead;
	ImgHead head;
	BYTE *arraybuffer;
	ssize_t image_data_size;

	CHECK_IMAGE(image);
	image_data_size = image->height * image->width * sizeof(RGBA_8888);
	arraybuffer = (BYTE *)calloc((size_t)1, sizeof(AbHead) + sizeof(ImgHead) + image_data_size);
	if (! arraybuffer) {
		syslog_error("Allocate memory for image_encode.");
		return NULL;
	}

	// 1. encode abhead
	abhead.t[0] = 'a';
	abhead.t[1] = 'b';
	abhead.len = sizeof(ImgHead) + image_data_size;
	_abhead_encode(&abhead, arraybuffer);

	// 2. encode image head
	head.h = image->height;
	head.w = image->width;
	head.c = sizeof(RGBA_8888);	// Channels
	head.opc = image->opc;
	_imghead_encode(&head, arraybuffer + sizeof(AbHead));	// skip AbHead

	// 3. encode image data
	memcpy(arraybuffer + sizeof(AbHead) + sizeof(ImgHead), image->base, image_data_size); // skip AbHead, ImgHead

	return arraybuffer;
}

IMAGE *image_decode(BYTE *buffer)
{
	// Buffer == AbHead + ImgHead + Image Data
	AbHead abhead;
	ImgHead head;
	IMAGE *image;
	int image_data_size;

	if (_abhead_decode(buffer, &abhead) != RET_OK) {
		syslog_error("Bad Array Buffer Head.\n");
		return NULL;
	}

	if (_imghead_decode(buffer + sizeof(AbHead), &head) != RET_OK) {
		syslog_error("Bad Image Head.\n");
		return NULL;
	}

	image_data_size = head.h * head.w * sizeof(RGBA_8888);
	if (abhead.len != (sizeof(ImgHead) + image_data_size)) {
		syslog_error("Bad Image Head.\n");
		return NULL;
	}

	image = image_create(head.h, head.w); CHECK_IMAGE(image);
	image->opc = head.opc;
	memcpy(image->base, buffer + sizeof(AbHead) + sizeof(ImgHead), image_data_size);

	return image;
}

IMAGE* image_recv(int fd)
{
	AbHead abhead;
	ImgHead imghead;
	IMAGE *image;
	ssize_t n, image_data_size;
	BYTE headbuf[sizeof(AbHead) + sizeof(ImgHead)], *databuf;

	if (read(fd, headbuf, sizeof(AbHead)) != sizeof(AbHead)) {
		syslog_error("Reading arraybuffer head.\n");
		return NULL;
	}

	// 1. Get abhead ?
	if (_abhead_decode(headbuf, &abhead) != RET_OK) {
		syslog_error("Bad AbHead: t = %c%c, len = %d, crc = %x .\n",
			abhead.t[0], abhead.t[1], abhead.len, abhead.crc);

		while (read(fd, headbuf, sizeof(headbuf)) > 0)
			; // Skip left dirty data ...

		return NULL;
	}

	// 2. Get image head ?
	if (read(fd, headbuf, sizeof(ImgHead)) != sizeof(ImgHead)) {
		syslog_error("Reading image head.\n");
		return NULL;
	}
	if (_imghead_decode(headbuf, &imghead) != RET_OK) {
		syslog_error("Bad ImgHead.\n");
		syslog_debug("ImgHead detail informatoin: HxWxC = %dx%dx%d, opc = %d.\n",
			 imghead.h, imghead.w, imghead.c, imghead.opc);
		while (read(fd, headbuf, sizeof(headbuf)) > 0)
			; // Skip left dirty data ...
		return NULL;
	}
	// syslog_debug("Good Image Head: HxWxC = %dx%dx%d, opc = %d.\n",
	// 	 imghead.h, imghead.w, imghead.c, imghead.opc);

	// 3. Get image data
	image = image_create(imghead.h, imghead.w); CHECK_IMAGE(image);
	image->opc = imghead.opc;	// Save RPC Method
	image_data_size = imghead.h * imghead.w * sizeof(RGBA_8888);
	databuf = (BYTE *)image->base;
	n = 0;
	while (n < image_data_size) {
		n += read(fd, databuf + n, image_data_size - n);
		if (n <= 0)
			break;
	}
	if (n != image_data_size) {
		syslog_error("Received %ld bytes image data -- expect %ld !!!\n", n, image_data_size);
		image_destroy(image);
		return NULL;
	}

	return image;
}

int image_send(int fd, IMAGE *image)
{
	ssize_t image_data_size;
	AbHead abhead;
	ImgHead imghead;
	BYTE buffer[sizeof(AbHead) + sizeof(ImgHead)];

	check_image(image);

	image_data_size = image->height * image->width * sizeof(RGBA_8888);

	// 1. encode abhead and send
	abhead.t[0] = 'a';
	abhead.t[1] = 'b';
	abhead.len = sizeof(ImgHead) + image_data_size;
	_abhead_encode(&abhead, buffer);
	if (write(fd, buffer, sizeof(AbHead)) != sizeof(AbHead)) {
		syslog_error("Write AbHead.");
		return RET_ERROR;
	}

	// 2. encode image head and send
	imghead.h = image->height;
	imghead.w = image->width;
	imghead.c = sizeof(RGBA_8888);	// Channels
	imghead.opc = image->opc;
	_imghead_encode(&imghead, buffer);
	if (write(fd, buffer, sizeof(ImgHead)) != sizeof(AbHead)) {
		syslog_error("Write ImgHead.");
		return RET_ERROR;
	}

	// 3. send data
	if (write(fd, image->base, image_data_size) != image_data_size) {
		syslog_error("Write image data.");
		return RET_ERROR;
	}

	return RET_OK;
}

