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

int image_service()
{
	IMAGE *image;

	image = image_read(0);
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
	image_write(1, image);

	image_destroy(image);
	return RET_OK;
}

void dataecho_help(char *cmd)
{
	printf("Usage: %s [option]\n", cmd);
	printf("    -h, --help                   Display this help.\n");

	exit(1);
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

