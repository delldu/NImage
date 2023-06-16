
/************************************************************************************
***
***	Copyright 2010-2020 Dell Du(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Sat Jul 31 14:19:59 HKT 2010
***
************************************************************************************/

#include "vector.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_MAGIC MAKE_FOURCC('V', 'E', 'C', 'T')

extern int math_gsbw(float sigma);

VECTOR* vector_create(int m)
{
    VECTOR* vec;

    if (m < 1) {
        syslog_error("Create vector with bad parameter.");
        return NULL;
    }

    vec = (VECTOR*)calloc((size_t)1, sizeof(VECTOR));
    if (!vec) {
        syslog_error("Allocate memeory.");
        return NULL;
    }

    vec->m = m;
    vec->ve = (float*)calloc((size_t)m, sizeof(float));
    if (!vec->ve) {
        syslog_error("Allocate memeory.");
        free(vec);
        return NULL;
    }

    vec->magic = VECTOR_MAGIC;

    return vec;
}

int vector_clear(VECTOR* vec)
{
    check_vector(vec);
    memset(vec->ve, 0, sizeof(float) * vec->m);
    return RET_OK;
}

void vector_destroy(VECTOR* v)
{
    if (!vector_valid(v))
        return;

    // FIX: Bug, Memorary leak !
    if (v->ve)
        free(v->ve);

    free(v);
}

void vector_print(VECTOR* v, char* format)
{
    int i, k;
    if (!vector_valid(v)) {
        syslog_error("Bad vector.");
        return;
    }

    for (i = 0; i < v->m; i++) {
        if (strchr(format, 'f')) // float
            printf(format, v->ve[i]);
        else {
            k = (int)v->ve[i];
            printf(format, k); // integer
        }
        printf("%s", (i < v->m - 1) ? ", " : "\n");
        // Too many data output on one line is not good idea !
        if ((i + 1) % 10 == 0)
            printf("\n");
    }
}

int vector_valid(VECTOR* v)
{
    return (v && v->m >= 1 && v->ve && v->magic == VECTOR_MAGIC);
}

int vector_cosine(VECTOR* v1, VECTOR* v2, float* res)
{
    int i;
    float s1, s2, s;

    if (!res) {
        syslog_error("Result pointer is null.");
        return RET_ERROR;
    }
    check_vector(v1);
    check_vector(v2);

    if (v1->m != v2->m) {
        syslog_error("Dimension of two vectors is not same.");
        return RET_ERROR;
    }

    s1 = s2 = s = 0;
    vector_foreach(v1, i)
    {
        s1 += v1->ve[i] * v1->ve[i];
        s2 += v2->ve[i] * v2->ve[i];
        s += v1->ve[i] * v2->ve[i];
    }
    if (ABS(s1) < MIN_FLOAT_NUMBER || ABS(s2) < MIN_FLOAT_NUMBER) {
        syslog_error("Two vectors are zero.");
        *res = 1.0f;
        return RET_OK;
    }

    *res = (float)(s / (sqrt(s1) * sqrt(s2)));

    return RET_OK;
}

int vector_normal(VECTOR* v)
{
    int i;
    float sum;

    check_vector(v);

    sum = vector_sum(v);
    if (ABS(sum) < MIN_FLOAT_NUMBER) // NO Need calculation more
        return RET_OK;

    vector_foreach(v, i) v->ve[i] /= sum;

    return RET_OK;
}

float vector_likeness(VECTOR* v1, VECTOR* v2)
{
    float d;
    vector_cosine(v1, v2, &d);
    return d;
}

float vector_sum(VECTOR* v)
{
    int i;
    float sum;

    if (!vector_valid(v)) {
        syslog_error("Bad vector.");
        return 0.0f;
    }

    sum = 0.0f;
    vector_foreach(v, i) sum += v->ve[i];

    return sum;
}

float vector_mean(VECTOR* v) { return vector_sum(v) / v->m; }

// Guass 1D kernel
VECTOR* vector_gskernel(float sigma)
{
    int j, dim;
    float d, g;
    VECTOR* vec;

    dim = math_gsbw(sigma) / 2;
    vec = vector_create(2 * dim + 1);
    CHECK_VECTOR(vec);
    d = sigma * sigma * 2.0f;
    for (j = 0; j <= dim; j++) {
        g = exp(-(1.0f * j * j) / d);
        vec->ve[dim + j] = g;
        vec->ve[dim - j] = g;
    }
    vector_normal(vec);

    return vec;
}
