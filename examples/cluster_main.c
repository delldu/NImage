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

void cluster_help(char *cmd)
{
	printf("Usage: %s [option]\n", cmd);
	printf("    -h, --help                   Display this help.\n");
	printf("    -i, --input <image file>     Input image name\n");
	// printf("    -o, --output <image file>    Output image name\n");
	printf("    -k, --class <number>         Cluster number, default is 32.\n");
	printf("    -r, --radius <number>        Instance distance, default is 2.\n");

	exit(1);
}

int cluster_main(int argc, char **argv)
{
	int optc;
	int option_index = 0;
	char *input = NULL;
	// char *output = NULL;
	int k = 32;
	int r = 2;

	struct option long_opts[] = {
		{ "help", 0, 0, 'h'},
		{ "input", 1, 0, 'i'},
		// { "output", 1, 0, 'o'},
		{ "class", 1, 0, 'k'},
		{ "radius", 1, 0, 'r'},

		{ 0, 0, 0, 0}

	};

	if (argc <= 2)
		cluster_help(argv[0]);
	
	while ((optc = getopt_long(argc, argv, "h i: o: k: r:", long_opts, &option_index)) != EOF) {
		switch (optc) {
		case 'i':	// Input
			input = optarg;
			break;
		// case 'o':	// Output
		// 	output = optarg;
		// 	break;
		case 'k':	// Class
			k = atoi(optarg);
			break;
		case 'r':	// Radius
			r = atoi(optarg);
			break;
		case 'h':	// help
		default:
			cluster_help(argv[0]);
			break;
	    	}
	}

	if (input) {
		test_crc16();

		IMAGE *image = image_load(input);
		image_show(image, "orig");
		shape_bestedge(image);

		image_show(image, "shape");
		
		// time_reset();
		// // image_gauss_filter(image, 2);
		// color_cluster_(image, k);
		// color_instance_(image, r);	// widow size (2*r + 1)*(2*r + 1)
		// time_spend("Color instance");

		// mask_show(image);
	}

	// MS -- Modify Section ?

	return 0;
}
