/************************************************************************************
***
***	Copyright 2017-2020 Dell(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Wed Jun 14 18:05:59 PDT 2017
***
************************************************************************************/

#include "image.h"

#define MASK_THRESHOLD 128
#define COLOR_CLUSTERS 32
#define BOARDER_THRESHOLD 5
#define MASK_ALPHA 0.0f

#define mask_foreach(img, i, j)           \
    for (i = 1; i < img->height - 1; i++) \
        for (j = 1; j < img->width - 1; j++)

static void __mask_binary(IMAGE* mask, int debug)
{
    int i, j;

    image_foreach(mask, i, j)
    {
        // set 0 to box border
        if (i == 0 || j == 0 || i == mask->height - 1 || j == mask->width - 1)
            mask->ie[i][j].g = 0;
        else
            mask->ie[i][j].g = (mask->ie[i][j].g < MASK_THRESHOLD) ? 0 : 255;
    }

    if (debug) {
        image_foreach(mask, i, j)
        {
            mask->ie[i][j].r = mask->ie[i][j].g;
            mask->ie[i][j].b = mask->ie[i][j].g;
        }
    }
}

static inline int __mask_border(IMAGE* mask, int i, int j)
{
    int k;
    static int nb[4][2] = { { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 0 } };

    if (mask->ie[i][j].g != 255)
        return 0;

    // current is 1
    for (k = 0; k < 4; k++) {
        if (mask->ie[i + nb[k][0]][j + nb[k][1]].g == 0)
            return 1;
    }

    return 0;
}

// neighbours
static inline int __mask_4conn(IMAGE* mask, int i, int j)
{
    int k, sum;
    static int nb[4][2] = { { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 0 } };

    sum = 0;
    for (k = 0; k < 4; k++) {
        if (mask->ie[i + nb[k][0]][j + nb[k][1]].g != 0)
            sum++;
    }

    return sum;
}

// Mask will clone point
static inline int __mask_cloner(IMAGE* mask, int i, int j)
{
    return (mask->ie[i][j].g == 255);
}

static void __mask_finetune(IMAGE* mask, IMAGE* src, int debug)
{
    int i, j, k, count[COLOR_CLUSTERS];

    __mask_binary(mask, debug);

    color_cluster(src, COLOR_CLUSTERS); // not update

    // Calculate border colors
    memset(count, 0, COLOR_CLUSTERS * sizeof(int));
    mask_foreach(mask, i, j)
    {
        if (!__mask_border(mask, i, j))
            continue;
        count[src->ie[i][j].a]++;
    }

    // Fast delete border's blocks
    for (k = 0; k < COLOR_CLUSTERS; k++) {
        if (count[k] < BOARDER_THRESHOLD)
            continue;
        mask_foreach(src, i, j)
        {
            // mask remove
            if (src->ie[i][j].a == k)
                mask->ie[i][j].r = mask->ie[i][j].g = mask->ie[i][j].b = 0;
        }
    }

    // Delete isolate points
    mask_foreach(mask, i, j)
    {
        if (!__mask_cloner(mask, i, j))
            continue;

        // current == 255
        k = __mask_4conn(mask, i, j);
        if (k <= 1) // mask remove
            mask->ie[i][j].r = mask->ie[i][j].g = mask->ie[i][j].b = 0;
    }

    // Fill isolate holes
    mask_foreach(mask, i, j)
    {
        if (__mask_cloner(mask, i, j))
            continue;

        // ==> current == 0
        k = __mask_4conn(mask, i, j);
        if (k >= 3) // mask update
            mask->ie[i][j].r = mask->ie[i][j].g = mask->ie[i][j].b = 255;
    }
}

int image_blend(IMAGE* src, IMAGE* mask, IMAGE* dst, int top, int left,
    int debug)
{
    int i, j, mask_is_null = 0;
    float d;

    check_image(src);
    check_image(dst);

    if (debug) {
        time_reset();
    }

    color_cluster(src, COLOR_CLUSTERS);
    if (mask == NULL) {
        mask_is_null = 1;
        mask = image_create(src->height, src->width);
        check_image(mask);
        image_foreach(mask, i, j)
        {
            if (i == 0 || j == 0 || i == mask->height - 1 || j == mask->width - 1)
                mask->ie[i][j].r = mask->ie[i][j].g = mask->ie[i][j].b = 0;
            else
                mask->ie[i][j].r = mask->ie[i][j].g = mask->ie[i][j].b = 255;
        }
    }

    __mask_finetune(mask, src, debug);

    if (debug) {
        image_foreach(mask, i, j)
        {
            if (__mask_cloner(mask, i, j))
                continue;
            src->ie[i][j].r = 0;
            src->ie[i][j].g = 0;
            src->ie[i][j].b = 0;
        }
    }

    image_foreach(mask, i, j)
    {
        if (!__mask_cloner(mask, i, j))
            continue;

        if (image_outdoor(dst, i, top, j, left)) {
            // syslog_error("Out of target image");
            continue;
        }

        if (__mask_border(mask, i, j)) {
            d = MASK_ALPHA * src->ie[i][j].r + (1 - MASK_ALPHA) * dst->ie[i + top][j + left].r;
            dst->ie[i + top][j + left].r = (BYTE)d;

            d = MASK_ALPHA * src->ie[i][j].g + (1 - MASK_ALPHA) * dst->ie[i + top][j + left].g;
            dst->ie[i + top][j + left].g = (BYTE)d;

            d = MASK_ALPHA * src->ie[i][j].b + (1 - MASK_ALPHA) * dst->ie[i + top][j + left].b;
            dst->ie[i + top][j + left].b = (BYTE)d;
        } else {
            dst->ie[i + top][j + left].r = src->ie[i][j].r;
            dst->ie[i + top][j + left].g = src->ie[i][j].g;
            dst->ie[i + top][j + left].b = src->ie[i][j].b;
        }
    }

    if (debug) {
        time_spend("Color cluster blending");
    }

    if (mask_is_null)
        image_destroy(mask);

    return RET_OK;
}

#define SEAM_LINE_THRESHOLD 2

// a: -1, b: 0,  c: 1
static int __abc_index(float a, float b, float c)
{
    if (a < b)
        return (a < c) ? -1 : 1;

    // a >= b
    return (b < c) ? 0 : 1;
}

// a: -1, b: 0
static int __ab_index(float a, float b) { return (a < b) ? -1 : 0; }

// b: 0, c: 1
static int __bc_index(float b, float c) { return (b < c) ? 0 : 1; }

// Dynamic programming
int* seam_program(MATRIX* mat, int debug)
{
    int i, j, index, besti, *C;
    float a, b, c, min;
    MATRIX *t, *loc; // location for best way

    CHECK_MATRIX(mat);

    t = matrix_create(mat->m, mat->n);
    CHECK_MATRIX(t);
    loc = matrix_create(mat->m, mat->n);
    CHECK_MATRIX(loc);
    C = (int*)calloc((size_t)mat->n, sizeof(int)); // CHECK_POINT(C);

    // Step 1. First column
    for (i = 0; i < mat->m; i++)
        t->me[i][0] = mat->me[i][0];

    // Step 2. t[][j], j >= 2,  Second and next columns
    for (j = 1; j < mat->n; j++) {
        // a) top row, i == 0
        i = 0;
        b = t->me[i][j - 1] + mat->me[i][j];
        c = t->me[i + 1][j - 1] + mat->me[i][j];
        index = __bc_index(b, c);
        if (index == 0) { // b solution
            t->me[i][j] = b;
            loc->me[i][j] = i;
        } else { // c solution
            t->me[i][j] = c;
            loc->me[i][j] = i + 1;
        }

        // b) midle rows
        for (i = 1; i < mat->m - 1; i++) {
            a = t->me[i - 1][j - 1] + mat->me[i][j];
            b = t->me[i][j - 1] + mat->me[i][j];
            c = t->me[i + 1][j - 1] + mat->me[i][j];
            index = __abc_index(a, b, c);
            if (index == -1) { // a
                t->me[i][j] = a;
                loc->me[i][j] = i - 1;
            } else if (index == 0) { // b
                t->me[i][j] = b;
                loc->me[i][j] = i;
            } else { // c
                t->me[i][j] = c;
                loc->me[i][j] = i + 1;
            }
        }

        // c) bottom row,  i == mat->m - 1
        i = mat->m - 1;
        a = t->me[i - 1][j - 1] + mat->me[i][j];
        b = t->me[i][j - 1] + mat->me[i][j];
        index = __ab_index(a, b);
        if (index == -1) {
            t->me[i][j] = a;
            loc->me[i][j] = i - 1;
        } else {
            t->me[i][j] = b;
            loc->me[i][j] = i;
        }
    }

    // Step 3. Check Last row to find best way
    besti = 0;
    min = t->me[0][t->n - 1];
    for (i = 1; i < mat->m; i++) {
        if (min > t->me[i][t->n - 1]) {
            min = t->me[i][t->n - 1];
            besti = i;
        }
    }

    if (debug) {
        //      printf("Seam Raw Matrix:\n");
        //      matrix_print(mat, "%lf");

        //      printf("Time Cost Matrix:\n");
        //      matrix_print(t, "%lf");

        printf("Last Best Path: %d, Min Cost Time: %lf\n", besti, min);
    }
    // Set best curve to C
    i = besti;
    C[mat->n - 1] = i;
    for (j = mat->n - 1; j >= 1; j--) {
        i = (int)loc->me[i][j];
        C[j - 1] = i;
    }

    if (debug) {
        for (j = 0; j < mat->n - 1; j++)
            printf("%d ", C[j]);
        printf("\n");
    }

    matrix_destroy(loc);
    matrix_destroy(t);

    return C;
}

// Seam a, b and return best seam line
// -- make sure rect_a, rect_b size is same
// mode: 0-3
int* image_seampath(IMAGE* image_a, RECT* rect_a, IMAGE* image_b, RECT* rect_b,
    int mode)
{
    MATRIX* mat; // seam matrix
    IMAGE* mask;
    float d, r2, x2, slope, bias;
    int i, j, a, b, c, *line; // seam line;

    CHECK_IMAGE(image_a);
    CHECK_IMAGE(image_b);

    mat = matrix_create(rect_a->h, rect_a->w);
    CHECK_MATRIX(mat);
    mask = image_create(rect_a->h, rect_a->w);
    CHECK_IMAGE(mask);

    switch (mode) {
    case 0: // A, Center = (h, w)
        matrix_foreach(mat, i, j)
        {
            // r2 = (i - rect_a->h)*(i - rect_a->h) + (j - rect_a->w)*(j - rect_a->w);
            r2 = 1.0f;
            a = (image_a->ie[i + rect_a->r][j + rect_a->c].r - image_b->ie[i + rect_b->r][j + rect_b->c].r);
            b = (image_a->ie[i + rect_a->r][j + rect_a->c].g - image_b->ie[i + rect_b->r][j + rect_b->c].g);
            c = (image_a->ie[i + rect_a->r][j + rect_a->c].b - image_b->ie[i + rect_b->r][j + rect_b->c].b);
            x2 = a * a + b * b + c * c;
            mat->me[i][j] = r2 * x2;
        }
        break;
    case 1: // B, Center = (h, 0)
        matrix_foreach(mat, i, j)
        {
            // r2 = (i - rect_a->h)*(i - rect_a->h) + j*j;
            r2 = 1.0f;
            a = (image_a->ie[i + rect_a->r][j + rect_a->c].r - image_b->ie[i + rect_b->r][j + rect_b->c].r);
            b = (image_a->ie[i + rect_a->r][j + rect_a->c].g - image_b->ie[i + rect_b->r][j + rect_b->c].g);
            c = (image_a->ie[i + rect_a->r][j + rect_a->c].b - image_b->ie[i + rect_b->r][j + rect_b->c].b);
            x2 = a * a + b * b + c * c;
            mat->me[i][j] = r2 * x2;
        }
        break;
    case 2: // C, Center = (0, w)
        matrix_foreach(mat, i, j)
        {
            // r2 = i * i + (j - rect_a->w)*(j - rect_a->w);
            r2 = 1.0f;
            a = (image_a->ie[i + rect_a->r][j + rect_a->c].r - image_b->ie[i + rect_b->r][j + rect_b->c].r);
            b = (image_a->ie[i + rect_a->r][j + rect_a->c].g - image_b->ie[i + rect_b->r][j + rect_b->c].g);
            c = (image_a->ie[i + rect_a->r][j + rect_a->c].b - image_b->ie[i + rect_b->r][j + rect_b->c].b);
            x2 = a * a + b * b + c * c;
            mat->me[i][j] = r2 * x2;
        }
        break;
    case 3: // D, Center = (0, 0)
        matrix_foreach(mat, i, j)
        {
            // r2 = i*i + j*j;
            r2 = 1.0f;
            a = (image_a->ie[i + rect_a->r][j + rect_a->c].r - image_b->ie[i + rect_b->r][j + rect_b->c].r);
            b = (image_a->ie[i + rect_a->r][j + rect_a->c].g - image_b->ie[i + rect_b->r][j + rect_b->c].g);
            c = (image_a->ie[i + rect_a->r][j + rect_a->c].b - image_b->ie[i + rect_b->r][j + rect_b->c].b);
            x2 = a * a + b * b + c * c;
            mat->me[i][j] = r2 * x2;
        }
        break;
    default:
        break;
    }

    line = seam_program(mat, 0);
    if (line == NULL) {
        syslog_error("Find best seam line.");
        matrix_destroy(mat);
        return NULL;
    }
    // Need to line correct ?
    // ... (r1, c1) -- (r2, c2) --> k = (r2 - r1)/(c2-c1), b = r2 - k*c2;
    switch (mode) {
    case 0: // A, Center = (h, w)
        b = mat->m - 1 - line[mat->n - 1];
        if (b >= SEAM_LINE_THRESHOLD) {
            a = MAX((mat->n - b), 0);
            c = line[a];
            slope = mat->m - 1;
            slope -= c;
            slope /= b;
            bias = mat->m - 1 - slope * (mat->n - 1);

            for (j = a; j < mat->n - 1; j++) {
                d = slope * j + bias;
                line[j] = CLAMP((int)d, 0, (mat->m - 1));
            }
        }
        break;
    case 1: // B, Center = (h, 0)
        b = mat->m - 1 - line[0];
        if (b >= SEAM_LINE_THRESHOLD) {
            a = MIN((mat->n - 1), b);
            c = line[a];
            slope = c;
            slope -= (mat->m - 1);
            slope /= b;
            bias = mat->m - 1;
            for (j = 0; j < a; j++) {
                d = slope * j + bias;
                line[j] = CLAMP((int)d, 0, (mat->m - 1));
            }
        }
        break;
    case 2: // C, Center = (0, w)
        b = line[mat->n - 1];
        if (b >= SEAM_LINE_THRESHOLD) {
            a = MAX((mat->n - b), 0);
            c = line[a];
            slope = -1.0f * c / b;
            bias = -slope * (mat->n - 1);

            for (j = a; j < mat->n - 1; j++) {
                d = slope * j + bias;
                line[j] = CLAMP((int)d, 0, (mat->m - 1));
            }
        }
        break;
    case 3: // D, Center = (0, 0)
        b = line[0];
        if (b >= SEAM_LINE_THRESHOLD) {
            a = MIN((mat->n - 1), b);
            c = line[a];
            slope = 1.0f * c / b;
            bias = 0.0f;
            for (j = 0; j < a; j++) {
                d = slope * j + bias;
                line[j] = CLAMP((int)d, 0, (mat->m - 1));
            }
        }
        break;
    default:
        break;
    }

    matrix_destroy(mat);

    return line;
}

// Seam a, b and save result to mask
// -- make sure rect_a, rect_b size is same
// mode: 0-3
IMAGE* image_seammask(IMAGE* image_a, RECT* rect_a, IMAGE* image_b,
    RECT* rect_b, int mode)
{
    IMAGE* mask;
    int i, j, *line; // seam line;

    CHECK_IMAGE(image_a);
    CHECK_IMAGE(image_b);

    line = image_seampath(image_a, rect_a, image_b, rect_b, mode);
    if (!line)
        return NULL;

    mask = image_create(rect_a->h, rect_a->w);
    CHECK_IMAGE(mask);
    if (mode == 0 || mode == 1) {
        // Border Up: a, Down: b
        for (j = 0; j < rect_b->w; j++) {
            for (i = 0; i < rect_b->h; i++) {
                if (i <= line[j])
                    mask->ie[i][j].r = mask->ie[i][j].g = mask->ie[i][j].b = 255;
                else
                    mask->ie[i][j].r = mask->ie[i][j].g = mask->ie[i][j].b = 0;
            }
        }
    } else {
        // Border Up: b, Down: a
        for (j = 0; j < rect_b->w; j++) {
            for (i = 0; i < rect_b->h; i++) {
                if (i >= line[j])
                    mask->ie[i][j].r = mask->ie[i][j].g = mask->ie[i][j].b = 255;
                else
                    mask->ie[i][j].r = mask->ie[i][j].g = mask->ie[i][j].b = 0;
            }
        }
    }

    free(line);

    return mask;
}
