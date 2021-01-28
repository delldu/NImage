/************************************************************************************
#***
#***	File Author: Dell, Fri May 30 12:46:39 CST 2020
#***
#************************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include "image.h"

int gray_distance(int c1, int c2)
{
	BYTE r, g, b, rgb1, rgb2;

	r = RGB_R(c1); g = RGB_G(c1); b = RGB_B(c1);
	color_rgb2gray(r, g, b, &rgb1);

	r = RGB_R(c2); g = RGB_G(c2); b = RGB_B(c2);
	color_rgb2gray(r, g, b, &rgb2);

	return ABS((int)rgb1 - (int)rgb2);
}

// A--> B, color B with A 
int color_fill(char *A, char *B, int num)
{
	int i, j, min_j, min_d, d, color;

	IMAGE *imagea, *imageb;

	imagea = image_load(A); check_image(imagea);
	imageb = image_load(B); check_image(imageb);

	image_gauss_filter(imagea, 0.5);
	color_cluster(imagea, num);

	image_gauss_filter(imageb, 0.5);
	color_cluster(imageb, num);

	mask_show(imagea);
	mask_show(imageb);

	// Update B->kColors
	for (i = 0; i < num; i++) {
		min_j = i;
		min_d = gray_distance(imageb->KColors[i], imagea->KColors[i]);
		for (j = 0; j < num; j++) {
			d = gray_distance(imageb->KColors[i], imagea->KColors[j]);
			if (min_d > d) {
				min_j = j;
				min_d = d;
			}
		}
		imageb->KColors[i] = imagea->KColors[min_j];
	}

	image_foreach(imageb, i, j) {
		color = imageb->KColors[imageb->ie[i][j].a];

		d = RGB_INT(imageb->ie[i][j].r, imageb->ie[i][j].g, imageb->ie[i][j].b);
		min_d = gray_distance(d, color);

		// d = ABS(RGB_R(color) - imageb->ie[i][j].r);
		// min_d += d * d;
		// d = ABS(RGB_G(color) - imageb->ie[i][j].g);
		// min_d += d * d;
		// d = ABS(RGB_B(color) - imageb->ie[i][j].b);
		// min_d += d * d;

		// printf("min_d = %d\n", min_d);

		if (min_d < 256/num) {
			imageb->ie[i][j].r = RGB_R(color);
			imageb->ie[i][j].g = RGB_G(color);
			imageb->ie[i][j].b = RGB_B(color);
		}
	}

	image_show(imageb, "ColorB");

	image_destroy(imageb);
	image_destroy(imagea);

	return RET_OK;
}

void help(char *cmd)
{
	printf("Usage: %s [option]\n", cmd);
	printf("    -h, --help                   Display this help.\n");

	printf("    -A, --inputa <image file>     Input image A\n");
	printf("    -B, --inputb <image file>     Input image B\n");
	printf("    -k, --class <number>          Cluster number, default is 128.\n");

	exit(1);
}

int main(int argc, char **argv)
{
	int optc;
	int option_index = 0;
	int n_class = 128;
	char *A = NULL;
	char *B = NULL;

	struct option long_opts[] = {
		{ "help", 0, 0, 'h'},
		{ "inputa", 1, 0, 'A'},
		{ "inputb", 1, 0, 'B'},
		{ "class", 1, 0, 'k'},

		{ 0, 0, 0, 0}
	};

	while ((optc = getopt_long(argc, argv, "h A: B: k:", long_opts, &option_index)) != EOF) {
		switch (optc) {
		case 'A':
			A = optarg;
			break;
		case 'B':
			B = optarg;
			break;
		case 'k':
			n_class = atoi(optarg);
			break;

		case 'h':	// help
		default:
			help(argv[0]);
			break;
    	}
	}

	if (A && B && n_class > 0) {
		return color_fill(A, B, n_class);
	}

	help(argv[0]);

	return 0;
}

