
/************************************************************************************
***
***	Copyright 2010-2020 Dell Du(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Sat Jul 31 14:19:59 HKT 2010
***
************************************************************************************/

#include "common.h"

// flock ...
#include <sys/file.h>
#include <errno.h>

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

void time_reset()
{
	__system_ms_time = time_now();
}

void time_spend(char *prompt)
{
	syslog_info("%s spend %ld ms.", prompt, time_now() - __system_ms_time);

	time_reset();
}

// example: maxhw == 512, times == 8
void space_resize(int h, int w, int maxhw, int times, int *nh, int *nw)
{
	int m;
	float scale;

	m = MAX(h, w);
	if (maxhw > 0 && m > maxhw) {
		scale = 1.0 * maxhw / m;
		h = (int)(scale * h);
		w = (int)(scale * w);
	}

	h = (h + times - 1)/times;
	w = (w + times - 1)/times;

	*nh =  h * times;
	*nw =  w * times;
}

int lock(char *endpoint)
{
	int i, n, fd, rc;
	char filename[256];
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

	fd = open(filename, O_CREAT|O_RDWR, 0666);
	if ((rc = flock(fd, LOCK_EX|LOCK_NB))) {
		rc = (EWOULDBLOCK == errno)? 1 : 0; 
	} else {
		rc = 0;
	}

	return rc == 0;
}