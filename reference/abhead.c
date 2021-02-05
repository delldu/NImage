/************************************************************************************
#***
#***	File Author: Dell, Fri May 30 12:46:39 CST 2020
#***
#************************************************************************************/

#include "abhead.h"
#include "image.h"

#define CRC_CCITT_POLY 0x1021	// CRC-CCITT, polynormial 0x1021.
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

void abhead_init(AbHead * abhead)
{
	memset(abhead, 0, sizeof(AbHead));
	abhead->t[0] = 'a';
	abhead->t[1] = 'b';
	abhead->c = sizeof(RGBA_8888);
}

int valid_ab(BYTE * buf, size_t size)
{
	AbHead abhead;
	return abhead_decode(buf, &abhead) == RET_OK && (abhead.len + sizeof(AbHead) == size) ? 1 : 0;
}

int abhead_decode(BYTE * buf, AbHead * head)
{
	// t[2], len-32, crc --16
	head->t[0] = buf[0];
	head->t[1] = buf[1];

	head->len = MAKE_FOURCC(buf[2], buf[3], buf[4], buf[5]);
	head->b = MAKE_TWOCC(buf[6], buf[7]);
	head->c = MAKE_TWOCC(buf[8], buf[9]);
	head->h = MAKE_TWOCC(buf[10], buf[11]);
	head->w = MAKE_TWOCC(buf[12], buf[13]);
	head->opc = MAKE_TWOCC(buf[14], buf[15]);
	head->crc = MAKE_TWOCC(buf[16], buf[17]);

	return (head->t[0] == 'a' && head->t[1] == 'b' && head->len > 0 &&
			head->b > 0 && head->c > 0 && head->c <= 4 &&
			head->h > 0 && head->w > 0 && head->crc == __abhead_crc16(buf, 16)) ? RET_OK : RET_ERROR;
}

int abhead_encode(AbHead * head, BYTE * buf)
{
	buf[0] = head->t[0];
	buf[1] = head->t[1];

	buf[2] = GET_FOURCC1(head->len);
	buf[3] = GET_FOURCC2(head->len);
	buf[4] = GET_FOURCC3(head->len);
	buf[5] = GET_FOURCC4(head->len);

	buf[6] = GET_TWOCC1(head->b);
	buf[7] = GET_TWOCC2(head->b);

	buf[8] = GET_TWOCC1(head->c);
	buf[9] = GET_TWOCC2(head->c);

	buf[10] = GET_TWOCC1(head->h);
	buf[11] = GET_TWOCC2(head->h);

	buf[12] = GET_TWOCC1(head->w);
	buf[13] = GET_TWOCC2(head->w);

	buf[14] = GET_TWOCC1(head->opc);
	buf[15] = GET_TWOCC2(head->opc);

	head->crc = __abhead_crc16(buf, 16);
	buf[16] = GET_TWOCC1(head->crc);
	buf[17] = GET_TWOCC2(head->crc);

	return RET_OK;
}

