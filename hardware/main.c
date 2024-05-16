/************************************************************************************
***
***	Copyright 2020 Dell(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, 2020-11-22 13:18:11
***
************************************************************************************/

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <license.h>

void detect_hardware()
{
    Hardware h;

    get_hardware(&h);
    dump_hardware(&h);
}

void help(char* cmd)
{
    printf("This is an example for hardware detection\n");

    printf("Usage: %s [option] image_file\n", cmd);
    printf("    -h, --help                   Display this help.\n");

    exit(1);
}

// int check_license(char *fname);

int main(int argc, char** argv)
{
    int optc;
    int option_index = 0;

    struct option long_opts[] = {
        { "help", 0, 0, 'h' },
        { 0, 0, 0, 0 } };


    while ((optc = getopt_long(argc, argv, "h", long_opts, &option_index)) != EOF) {
        switch (optc) {
        case 'h': // help
        default:
            help(argv[0]);
            break;
        }
    }

    detect_hardware();
    check_license("hw.lic");

    return RET_OK;
}
