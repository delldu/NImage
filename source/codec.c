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

#define CRC_CCITT_POLY 0x1021	//CRC-CCITT, polynormial 0x1021.
// 0x31 0x32 0x33 0x34 0x35 0x36 ==> 0x20E4
// ab -- ArrayBuffer, AbHead == {'ab', 4 bytes len, crc16}, total 8 bytes
WORD __abhead_crc16(BYTE * buf, int size)
{
	int i;
	WORD crc;

	crc = 0;
	while (--size >= 0) {
		crc = crc ^ ((WORD) (*buf++ << 8));
		for (i = 0; i < 8; i++) {
			if (crc & 0x8000)
				crc = (crc << 1) ^ CRC_CCITT_POLY;
			else
				crc = crc << 1;
		}
	}

	return crc;
}

int __abhead_decode(BYTE * buf, AbHead *head)
{
	// t[2], len-32, crc --16
	head->t[0] = buf[0];
	head->t[1] = buf[1];
	head->len = MAKE_FOURCC(buf[2], buf[3], buf[4], buf[5]);
	head->h = WORD_FROM_BYTES(buf[6], buf[7]);
	head->w = WORD_FROM_BYTES(buf[8], buf[9]);
	head->c = WORD_FROM_BYTES(buf[10], buf[11]);
	head->opc = WORD_FROM_BYTES(buf[12], buf[13]);
	head->crc = WORD_FROM_BYTES(buf[14], buf[15]);
	return (head->len > 0 && head->t[0] == 'a' && head->t[1] == 'b' &&
			head->h > 0 && head->w > 0 && head->c > 0 && head->c <= 4 &&
			head->crc == __abhead_crc16(buf, 14)) ? RET_OK : RET_ERROR;
}

int __abhead_encode(AbHead * head, BYTE * buf)
{
	buf[0] = head->t[0];
	buf[1] = head->t[1];
	buf[2] = GET_FOURCC1(head->len);
	buf[3] = GET_FOURCC2(head->len);
	buf[4] = GET_FOURCC3(head->len);
	buf[5] = GET_FOURCC4(head->len);
	buf[6] = WORD_LOW(head->h);
	buf[7] = WORD_HI(head->h);
	buf[8] = WORD_LOW(head->w);
	buf[9] = WORD_HI(head->w);
	buf[10] = WORD_LOW(head->c);
	buf[11] = WORD_HI(head->c);
	buf[12] = WORD_LOW(head->opc);
	buf[13] = WORD_HI(head->opc);
	head->crc = __abhead_crc16(buf, 14);
	buf[14] = WORD_LOW(head->crc);
	buf[15] = WORD_HI(head->crc);

	return RET_OK;
}

int image_data_size(IMAGE *image)
{
	return image->height * image->width * sizeof(RGBA_8888);
}

IMAGE *image_recv(int fd)
{
	AbHead abhead;
	IMAGE *image;
	ssize_t n, data_size;
	BYTE headbuf[sizeof(AbHead)], *databuf;

	if (read(fd, headbuf, sizeof(AbHead)) != sizeof(AbHead)) {
		syslog_error("Reading arraybuffer head.\n");
		return NULL;
	}
	// 1. Get abhead ?
	if (__abhead_decode(headbuf, &abhead) != RET_OK) {
		syslog_error("Bad AbHead: t = %c%c, len = %d, crc = %x .\n", 
			abhead.t[0], abhead.t[1], abhead.len, abhead.crc);
		while (read(fd, headbuf, sizeof(headbuf)) > 0);	// Skip left dirty data ...

		return NULL;
	}
	// 2. Get image data
	image = image_create(abhead.h, abhead.w); CHECK_IMAGE(image);
	image->opc = abhead.opc;	// Save RPC Method
	data_size = image_data_size(image);
	databuf = (BYTE *) image->base;
	n = 0;
	while (n < data_size) {
		n += read(fd, databuf + n, data_size - n);
		if (n <= 0)
			break;
	}
	if (n != data_size) {
		syslog_error("Received %ld bytes image data -- expect %ld !!!\n", n, data_size);
		image_destroy(image);
		return NULL;
	}

	return image;
}

int image_send(int fd, IMAGE * image)
{
	ssize_t data_size;
	BYTE buffer[sizeof(AbHead)];

	check_image(image);

	data_size = image_data_size(image);

	// 1. encode abhead and send
	if (image_abhead_encode(image, buffer) != RET_OK) {
		syslog_error("Image AbHead encode.");
	}
	if (write(fd, buffer, sizeof(AbHead)) != sizeof(AbHead)) {
		syslog_error("Write AbHead.");
		return RET_ERROR;
	}

	// 2. send data
	if (write(fd, image->base, data_size) != data_size) {
		syslog_error("Write image data.");
		return RET_ERROR;
	}

	return RET_OK;
}

int image_abhead_encode(IMAGE *image, BYTE *buffer)
{
	AbHead t;
	ssize_t data_size;

	check_image(image);
	data_size = image_data_size(image);

	// encode abhead
	t.t[0] = 'a';
	t.t[1] = 'b';
	t.len = data_size;
	t.h = image->height;
	t.w = image->width;
	t.c = 4;
	t.opc = image->opc;

	return __abhead_encode(&t, buffer);
}

int image_abhead_decode(BYTE * buffer, AbHead *abhead)
{
	// Array Buffer == AbHead + Image Data
	return __abhead_decode(buffer, abhead);
}
