
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
TIME time_now() {
  TIME ms;
  struct timeval t;

  gettimeofday(&t, NULL);
  ms = t.tv_sec * 1000 + t.tv_usec / 1000;

  return ms;
}

void time_reset() { __system_ms_time = time_now(); }

void time_spend(char *prompt) {
  syslog_info("%s spend %ld ms.", prompt, time_now() - __system_ms_time);

  time_reset();
}

// example: maxhw == 512, times == 8
void space_resize(int h, int w, int maxhw, int times, int *nh, int *nw) {
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

int lock(char *endpoint) {
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

int file_size(char *filename) {
  struct stat s;
  return (stat(filename, &s) != 0 || !S_ISREG(s.st_mode)) ? -1 : s.st_size;
}

char *file_load(char *filename, int *size) {
  int fd;
  char *buf;

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

int file_save(char *filename, char *buf, int size) {
  int ret = RET_ERROR;
  int fd = open(filename, O_RDWR | O_CREAT, 0600);
  if (fd >= 0) {
    ret = (write(fd, buf, size) == size) ? RET_OK : RET_ERROR;
    close(fd);
  }
  return ret;
}

// For tar files
/* values used in typeflag field */

#define REGTYPE '0'   /* regular file */
#define AREGTYPE '\0' /* regular file */
#define LNKTYPE '1'   /* link */
#define SYMTYPE '2'   /* reserved */
#define CHRTYPE '3'   /* character special */
#define BLKTYPE '4'   /* block special */
#define DIRTYPE '5'   /* directory */
#define FIFOTYPE '6'  /* FIFO special */
#define CONTTYPE '7'  /* reserved */

/* GNU tar extensions */

#define GNUTYPE_DUMPDIR 'D'  /* file names from dumped directory */
#define GNUTYPE_LONGLINK 'K' /* long link name */
#define GNUTYPE_LONGNAME 'L' /* long file name */
#define GNUTYPE_MULTIVOL 'M' /* continuation of file from another volume */
#define GNUTYPE_NAMES 'N'    /* file name that does not fit into main hdr */
#define GNUTYPE_SPARSE 'S'   /* sparse file */
#define GNUTYPE_VOLHDR 'V'   /* tape/volume header */

/* tar header */

#define BLOCKSIZE 512
#define SHORTNAMESIZE 100

typedef struct {      /* byte offset */
  char name[100];     /*   0 */
  char mode[8];       /* 100 */
  char uid[8];        /* 108 */
  char gid[8];        /* 116 */
  char size[12];      /* 124 */
  char mtime[12];     /* 136 */
  char chksum[8];     /* 148 */
  char typeflag;      /* 156 */
  char linkname[100]; /* 157 */
  char magic[6];      /* 257 */
  char version[2];    /* 263 */
  char uname[32];     /* 265 */
  char gname[32];     /* 297 */
  char devmajor[8];   /* 329 */
  char devminor[8];   /* 337 */
  char prefix[155];   /* 345 */
                      /* 500 */
} tar_header_t;

union tar_buffer {
  char buffer[BLOCKSIZE];
  tar_header_t header;
};

int getoct(char *p, int width) {
  char c;
  int result = 0;

  while (width--) {
    c = *p++;
    if (c == 0)
      break;
    if (c == ' ')
      continue;
    if (c < '0' || c > '7')
      return -1;
    result = result * 8 + (c - '0');
  }
  return result;
}

char *load_fromtar(char *tar_filename, char *file_name, int *file_size) {
  gzFile gzfile;
  union tar_buffer buffer;
  int len, err, fsize;
  char typeflag, fname[BLOCKSIZE], *buf = NULL;

  gzfile = gzopen(tar_filename, "rb");
  if (gzfile == NULL) {
    syslog_error("Open file %s.", tar_filename);
    return NULL;
  }

  while (1) {
    len = gzread(gzfile, &buffer, BLOCKSIZE);
    /*
     * Always expect full blocks and valid file name
     */
    if (len != BLOCKSIZE || buffer.header.name[0] == 0) {
      syslog_error("%s.", gzerror(gzfile, &err));
      break;
    }
    typeflag = buffer.header.typeflag;
    if (typeflag != REGTYPE && typeflag != AREGTYPE &&
        typeflag != GNUTYPE_LONGLINK && typeflag != GNUTYPE_LONGNAME)
      continue;

    if ((fsize = getoct(buffer.header.size, 12)) == -1) // impossiable
      break;

    // Normal file ?
    strncpy(fname, buffer.header.name, SHORTNAMESIZE);
    if (fname[SHORTNAMESIZE - 1] != 0)
      fname[SHORTNAMESIZE] = 0;

    if (typeflag == GNUTYPE_LONGLINK || typeflag == GNUTYPE_LONGNAME) {
      // long file name, read more ...
      if (gzread(gzfile, fname, BLOCKSIZE) != BLOCKSIZE) {
        syslog_error("%s.", gzerror(gzfile, &err));
        break;
      }
    }

    if (strcmp(fname, file_name) == 0) {
      // Found expect file ...
      buf = malloc(fsize + 1);
      if (buf != NULL) {
        *file_size = gzread(gzfile, buf, fsize);
      } else {
        syslog_error("Allocate memory.");
      }
      break;
    } else {
      // Skip not wanted file ...
      while (fsize > 0) {
        len = gzread(gzfile, &buffer, BLOCKSIZE);
        if (len != BLOCKSIZE) {
          syslog_error("%s.", gzerror(gzfile, &err));
          fsize = 0; // force quit
        } else {
          fsize -= len;
        }
      }
    } // end file data read loop
  }

  gzclose(gzfile);

  return buf;
}
