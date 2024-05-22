
/************************************************************************************
***
***	Copyright 2024 Dell Du(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Wed 15 May 2024 11:50:15 PM CST
***
************************************************************************************/

#ifndef _LICENSE_H
#define _LICENSE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "common.h"

typedef struct {
    int is_docker;

    // CPU 0
    int cpu_count;
    char cpu_name[256];

    // Main board
    char board_name[256];

    // GPU 0
    int gpu_count;
    char gpu_name[256];
    char cuda_version[256]; 

    // Ethernet 0
    char mac_address[18];

    // Time
    time_t date;
    int expire; // days
} Hardware;

int get_hardware(Hardware *h);
void dump_hardware(Hardware *h);
int check_license(char *fname);

#if defined(__cplusplus)
}
#endif
#endif // _LICENSE_H
