
/************************************************************************************
***
***	Copyright 2010-2020 Dell Du(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Sat Jul 31 14:19:59 HKT 2010
***
************************************************************************************/

#include "common.h"

// flock ...
#include <errno.h>
#include <sys/file.h>

// stat ...
#include <sys/stat.h>
#include <zlib.h>

static TIME __system_ms_time;


// return ms
TIME time_now()
{
    TIME ms;
    struct timeval t;

    gettimeofday(&t, NULL);
    ms = t.tv_sec * 1000 + t.tv_usec / 1000;

    return ms;
}

void time_reset() { __system_ms_time = time_now(); }

void time_spend(char* prompt)
{
    syslog_info("%s spend %ld ms.", prompt, time_now() - __system_ms_time);

    time_reset();
}

// example: maxhw == 512, times == 8
void space_resize(int h, int w, int maxhw, int times, int* nh, int* nw)
{
    int m;
    float scale;
    // Make sure nh and nw <= maxhw,
    m = MAX(h, w);
    if (maxhw > 0 && m > maxhw) {
        scale = 1.0 * maxhw / m;
        h = (int)(scale * h);
        w = (int)(scale * w);
    }

    h = (h + times - 1) / times;
    w = (w + times - 1) / times;

    *nh = h * times;
    *nw = w * times;
}

int file_lock(char* endpoint)
{
    int i, n, fd, rc;
    char filename[512];
    char tempstr[256];

    n = strlen(endpoint);
    for (i = 0; i < n && i < (int)sizeof(tempstr) - 1; i++) {
        if (endpoint[i] == '/' || endpoint[i] == ':')
            tempstr[i] = '_';
        else
            tempstr[i] = endpoint[i];
    }
    tempstr[i] = '\0';

    snprintf(filename, sizeof(filename), "/tmp/%s.lock", tempstr);

    fd = open(filename, O_CREAT | O_RDWR, 0666);
    if ((rc = flock(fd, LOCK_EX | LOCK_NB))) {
        rc = (EWOULDBLOCK == errno) ? 1 : 0;
    } else {
        rc = 0;
    }

    return rc == 0;
}

int file_exist(char* filename) { return (access(filename, F_OK) == 0); }

int file_size(char* filename)
{
    struct stat s;
    return (stat(filename, &s) != 0 || !S_ISREG(s.st_mode)) ? -1 : s.st_size;
}

char* file_load(char *filename, int *size)
{
    int fd;
    char* buf;

    *size = file_size(filename);
    if (*size < 0)
        return NULL;

    fd = open(filename, O_RDONLY);
    if (fd >= 0) {
        buf = malloc(*size + 1);
        if (buf != NULL) {
            *size = read(fd, buf, *size);
            buf[*size] = '\0'; // force set '\0' for strings
        } else {
            syslog_error("Allocate memory.");
        }
        close(fd);

        // OK
        return buf;
    }
    // fd < 0
    syslog_error("Loading file (%s).", filename);
    return NULL;
}

int file_save(char* filename, char* buf, int size)
{
    int ret = RET_ERROR;
    int fd = open(filename, O_RDWR | O_CREAT, 0600);
    if (fd >= 0) {
        ret = (write(fd, buf, size) == size) ? RET_OK : RET_ERROR;
        close(fd);
    }
    return ret;
}

int file_chown(char* dfile, char* sfile)
{
    struct stat s;
    if (stat(sfile, &s) != 0 || !(S_ISREG(s.st_mode) || S_ISDIR(s.st_mode))) {
        syslog_error("'%s' is not regular file or folder.", sfile);
        return RET_ERROR;
    }
    return (chown(dfile, s.st_uid, s.st_gid) == 0) ? RET_OK : RET_ERROR;
}

int make_dir(char* dirname)
{
    int ret = RET_OK;

    if (access(dirname, W_OK) != 0)
        ret = (mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
            ? RET_OK
            : RET_ERROR;
    if (ret != RET_OK)
        syslog_error("Create dir '%s'.", dirname);

    return ret;
}
