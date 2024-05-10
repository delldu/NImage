
/************************************************************************************
***
***	Copyright 2010-2020 Dell Du(18588220928@163.com), All Rights Reserved.
***
***	File Author: Dell, Sat Jul 31 14:19:59 HKT 2010
***
************************************************************************************/
#include "common.h"
#include "image.h"

#define MAYBE_SKELETON_COLOR 128

#define MAX_POINT_NO 8192

typedef struct {
    int r, c;
} DOT;
typedef struct {
    int count;
    DOT dot[MAX_POINT_NO];
} DOTS;

static DOTS __global_dot_set;

DOTS* dot_set() { return &__global_dot_set; }

void dot_put(int r, int c)
{
    DOTS* dots = dot_set();
    if (dots->count < MAX_POINT_NO) {
        dots->dot[dots->count].r = r;
        dots->dot[dots->count].c = c;
        dots->count++;
    } else {
        syslog_debug("Too many dots.");
    }
}

static int dot_compare(const void * a, const void * b)
{
    DOT *dot_a = (DOT *)a;
    DOT *dot_b = (DOT *)b;

    if (dot_a->r == dot_b->r)
        return dot_a->c - dot_b->c;

    return dot_a->r - dot_b->r;
}

void dot_sort()
{
    DOTS *s = dot_set();
    qsort(s->dot, sizeof(DOT), s->count, dot_compare);
}


static int __higher_cross(char orgb, IMAGE* img, int i, int j, int mid_thr)
{
    int k, a, b, delta;
    static int nb[4][2] = { { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 0 } };

    delta = mid_thr / 32 + 1;

    if (image_getvalue(img, orgb, i, j) < mid_thr + delta)
        return 0;

    a = b = 0;
    for (k = 0; k < 4; k++) {
        if (image_getvalue(img, orgb, i + nb[k][0], j + nb[k][1]) < mid_thr)
            a++;
        if (image_getvalue(img, orgb, i + nb[k][0], j + nb[k][1]) > mid_thr)
            b++;
    }
    return (a > 0 && b > 0) ? 1 : 0; // cell split nb with different world
}

// gm -- gradient m. ...
static int __canny_histsumv(IMAGE* img, float hirate)
{
    int i, j, hist[256], maxgm, hcount;

    for (j = 0; j < 256; j++)
        hist[j] = 0;

    for (i = 1; i < (int)img->height - 1; i++) {
        for (j = 1; j < (int)img->width - 1; j++) {
            if (img->ie[i][j].b == 128)
                hist[img->ie[i][j].g]++;
        }
    }
    for (i = maxgm = 0, j = 1; j < 256; j++) {
        if (hist[j])
            maxgm = j;
        i += hist[j];
    }
    hcount = (int)(hirate * i + 0.5);
    // Count max gradient (more than hcount)
    for (i = 0, j = 1; j < maxgm && i < hcount; j++)
        i += hist[j];
    return j;
}

// 8 neighbour trace
static void __canny_8nbtrace(IMAGE* img, int i, int j, int lowthreashold)
{
    // 8- neighbour coordinations
    static int jnbs[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
    static int inbs[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };

    if (i < 1 || i >= (int)img->height - 1 || j < 1 || j >= (int)img->width - 1)
        return;

    int r, c, k;
    for (k = 0; k < 8; k++) {
        r = i + inbs[k];
        c = j + jnbs[k];
        if (img->ie[r][c].b == 128 && img->ie[r][c].g >= lowthreashold) {
            img->ie[r][c].b = 255;
            dot_put(r, c);
            __canny_8nbtrace(img, r, c, lowthreashold);
        }
    }
}

static void __canny_edgetrace(IMAGE* img, float hr, float lr)
{
    int i, j, k;
    int hithreshold = __canny_histsumv(img, hr); // High Threshold
    int lothreshold = (int)(hithreshold * lr + 0.5);
    DOTS* dots = dot_set();
    char* p = getenv("CANNY_EDGE_MIN_DOTS");
    int min_edge_dots = (p) ? atoi(p) : 4;

    // printf("CANNY_EDGE_MIN_DOTS = %d\n", min_edge_dots);

    for (i = 1; i < (int)img->height - 1; i++) {
        for (j = 1; j < (int)img->width - 1; j++) {
            if (img->ie[i][j].b != 128 || img->ie[i][j].g < hithreshold)
                continue;
            dots->count = 0; // clear dots
            img->ie[i][j].b = 255;
            dot_put(i, j);
            __canny_8nbtrace(img, i, j, lothreshold);
            if (dots->count < min_edge_dots) {
                for (k = 0; k < dots->count; k++)
                    img->ie[dots->dot[k].r][dots->dot[k].c].b = 0;
            }
        }
    }

    // Trace end, other points is impossiable to be border
    image_foreach(img, i, j)
    {
        if (img->ie[i][j].b != 255)
            img->ie[i][j].b = 0;
    }
}

static int __canny_gradient(IMAGE* img)
{
    int i, j, g1, g2, g3, g4, gx, gy;
    float weight, dt1, dt2;

    for (i = 1; i < (int)img->height - 1; i++) {
        for (j = 1; j < (int)img->width - 1; j++) {
            gx = img->ie[i][j + 1].r - img->ie[i][j - 1].r;
            gy = img->ie[i + 1][j].r - img->ie[i - 1][j].r;
            img->ie[i][j].g = (BYTE)(sqrt((float)(gx * gx + gy * gy) / 2.0f)); // <= 255
        }
    }

    for (i = 1; i < (int)img->height - 1; i++) {
        for (j = 1; j < (int)img->width - 1; j++) {
            img->ie[i][j].b = 0;
            if (img->ie[i][j].g == 0)
                continue;

            gx = img->ie[i][j + 1].r - img->ie[i][j - 1].r;
            gy = img->ie[i + 1][j].r - img->ie[i - 1][j].r;

            // gm != 0
            if (fabs((float)gy) > fabs((float)gx)) {
                weight = fabs((float)gx) / fabs((float)gy);
                g2 = img->ie[i - 1][j].g;
                g4 = img->ie[i + 1][j].g;
                if (gx * gy > 0) {
                    g1 = img->ie[i - 1][j - 1].g; //   /
                    g3 = img->ie[i + 1][j + 1].g;
                } else {
                    g1 = img->ie[i - 1][j + 1].g; //
                    g3 = img->ie[i + 1][j - 1].g;
                }
            } else {
                weight = fabs((float)gy) / fabs((float)gx);
                g2 = img->ie[i][j + 1].g;
                g4 = img->ie[i][j - 1].g;
                if (gx * gy > 0) {
                    g1 = img->ie[i + 1][j + 1].g; //   /
                    g3 = img->ie[i - 1][j - 1].g;
                } else {
                    g1 = img->ie[i - 1][j + 1].g; //
                    g3 = img->ie[i + 1][j - 1].g;
                }
            }

            dt1 = weight * g1 + (1 - weight) * g2;
            dt2 = weight * g3 + (1 - weight) * g4;
            // if(gm->me[i][j] >= dt1 && gm->me[i][j] >= dt2)
            if (img->ie[i][j].g > dt1 && img->ie[i][j].g > dt2)
                img->ie[i][j].b = 128; // maybe is border point
        }
    }

    return RET_OK;
}

// shape_canny('R', image, 0.9, 0.75), Canny edge
static int __canny_edge_detect(IMAGE* img, float hr, float lr)
{
    check_image(img);

    // data storage format: R -- raw data,  G-- gradient m ..., B -- edge  !!!

    if (__canny_gradient(img) != RET_OK) {
        syslog_error("Canny gradient.");
        return RET_ERROR;
    }

    __canny_edgetrace(img, hr, lr);

    return RET_OK;
}


static int __contour_border(MATRIX* mat, int r, int c)
{
    int di, dj, k;
    static int d3[4][2] = { { -1, 0 }, { 0, -1 }, { 0, 1 }, { 1, 0 } }; // 3d.txt

    if (ABS(mat->me[r][c]) <= MIN_FLOAT_NUMBER) // Make sure (r, c) is not on the border
        return 0;

    for (k = 0; k < 4; k++) {
        di = d3[k][0];
        dj = d3[k][1];
        if (ABS(mat->me[r + di][c + dj]) <= MIN_FLOAT_NUMBER)
            return 1;
    }
    return 0;
}

static int __matrix_contour(MATRIX* mat)
{
    int i, j, count, height;
    MATRIX* copy;

    check_matrix(mat);
    copy = matrix_copy(mat);
    check_matrix(copy);

    matrix_clear(mat);
    height = 1;
    do { // repeat erode
        count = 0;
        for (i = height; i < copy->m - height; i++) {
            for (j = height; j < copy->n - height; j++) {
                if (__contour_border(copy, i, j)) {
                    mat->me[i][j] = height;
                    count++;
                }
            }
        }
        if (count > 0) { // ==> height line exists
            for (i = height; i < copy->m - height; i++) {
                for (j = height; j < copy->n - height; j++) {
                    if (mat->me[i][j] == height)
                        copy->me[i][j] = 0;
                }
            }
            height++;
        }
    } while (count > 0);

    matrix_destroy(copy);

    return height;
}

// Carte to polar coordinate
void math_topolar(int x, int y, float* r, float* a)
{
    float dr, da;

    if (x == 0) {
        if (y >= 0) {
            *a = 90;
            *r = y;
        } else {
            *a = 270;
            *r = -y;
        }
        return;
    }
    // x != 0
    dr = sqrt(x * x + y * y) + 0.5000;
    da = asin(1.0000 * y / dr) / 3.1415926 * 180;

    *r = dr;
    if (x > 0) { //  (x, y) in 1/4 section
        *a = da;
        if (y < 0)
            *a += 360;
    } else //  (x, y) in 2/3 section
        *a = 180 - da;
}

int math_arcindex(float a, int arcstep)
{
    int n;
    n = (2 * a + arcstep) / (2 * arcstep); // [a/arcstep + 1/2]
    if (n >= 360 / arcstep)
        n = 0;
    return n;
}

VECTOR* shape_vector(IMAGE* img, RECT* rect, int ndim)
{
    BYTE g;
    float d, r, a;
    int i, j, k, n, crow, ccol, arcstep;
    long long sum_all, sum_arc[360];
    VECTOR* vec;
    IMAGE* sub;

    CHECK_IMAGE(img);
    if (ndim > 360) {
        syslog_error("Bad dimension %d.", ndim);
        return NULL;
    }
    vec = vector_create(ndim);
    CHECK_VECTOR(vec);

    // Initialise
    arcstep = 360 / ndim;
    sum_all = 0;
    for (j = 0; j < ndim; j++)
        sum_arc[j] = 0;

    if (rect) {
        sub = image_subimg(img, rect);
        CHECK_IMAGE(sub);
        shape_bestedge(sub, 0.4, 0.8);
        image_mcenter(sub, 'A', &crow, &ccol);
        image_foreach(sub, i, j)
        {
            g = image_getvalue(sub, 'A', i, j);
            if (g == 0)
                continue;
            math_topolar(j - ccol, i - crow, &r, &a);
            n = math_arcindex(a, arcstep);
            k = r * g;
            sum_arc[n] += k;
            sum_all += k;
        }
        image_destroy(sub);
    } else {
        shape_bestedge(img, 0.4, 0.8);
        image_mcenter(img, 'A', &crow, &ccol);
        image_foreach(img, i, j)
        {
            g = image_getvalue(img, 'A', i, j);
            if (g == 0)
                continue;
            math_topolar(j - ccol, i - crow, &r, &a);
            n = math_arcindex(a, arcstep);
            k = r * g;
            sum_arc[n] += k;
            sum_all += k;
        }
    }

    n = 0;
    if (sum_all) {
        for (i = 0; i < vec->m; i++) {
            if (sum_arc[i] > sum_arc[n])
                n = i;
            d = (float)sum_arc[i] / sum_all;
            vec->ve[i] = d;
        }
    } else {
        syslog_error("All sum == 0.");
    }

#if 0
	d = arcstep;
	d *= n;
	d = 360 - d;
	printf("n = %d, (%d, %d, %.4f)\n", n, crow, ccol, d);
	//arcstep = 360 - arcstep;
	d /= 180;
	d *= MATH_PI;
	printf("arcstep = %.4f, sin() = %.2f, cos = %.2f\n", d, sin(d), cos(d));

	image_drawtext(img, crow, ccol, "Start", 0x00ffff);
	image_drawtext(img, crow - 100 * sin(d), ccol + 100 * cos(d), "Stop", 0x00ffff);

	image_drawline(img, crow, ccol, crow - 100 * sin(arcstep), ccol + 100 * cos(arcstep), 0x00ffff);
	image_drawline(img, crow, ccol, crow + 10, ccol - 10, 0x00ffff);
#endif

    return vec;
}

float shape_likeness(IMAGE* f, IMAGE* g, RECT* rect, int ndim)
{
    float avgd;
    VECTOR *fs, *gs;

    fs = shape_vector(f, rect, ndim);
    check_vector(fs);
    gs = shape_vector(g, rect, ndim);
    check_vector(gs);

    avgd = vector_likeness(fs, gs);

    vector_destroy(fs);
    vector_destroy(gs);

    return avgd;
}

int image_contour(IMAGE* img)
{
    int i, j, n;
    MATRIX* mat;

    check_image(img);
    mat = matrix_create(img->height, img->width);
    check_matrix(mat);

    // Convert image to bitmap on channel 'A'
    n = color_midval(img, 'A');

    image_foreach(img, i, j) mat->me[i][j] = (image_getvalue(img, 'A', i, j) < n) ? 0.0f : 255.0f;
    n = __matrix_contour(mat);

    // NOW contour results are saved in matrix mat
    matrix_foreach(mat, i, j)
    {
        n = (int)mat->me[i][j];
        if (n && (n % 5) == 0)
            img->ie[i][j].r = img->ie[i][j].g = img->ie[i][j].b = 255;
        else
            img->ie[i][j].r = img->ie[i][j].g = img->ie[i][j].b = 0;
    }
    matrix_destroy(mat);

    return RET_OK;
}

// Binary image thinning in-place with Zhang-Suen algorithm.
static int __thinning_step(IMAGE *image, int iter)
{
    int i, j;
    int need_check_more = 0;

    for (i = 1; i < image->height - 1; i++) {
        for (j = 1; j < image->width - 1; j++) {
            int p2 = image->ie[i - 1][j].a & 1;
            int p3 = image->ie[i - 1][j + 1].a & 1;
            int p4 = image->ie[i][j + 1].a & 1;
            int p5 = image->ie[i + 1][j + 1].a & 1;
            int p6 = image->ie[i + 1][j].a & 1;
            int p7 = image->ie[i + 1][j - 1].a & 1;
            int p8 = image->ie[i][j - 1].a & 1;
            int p9 = image->ie[i - 1][j - 1].a & 1;

            int A = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) 
                + (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) + (p6 == 0 && p7 == 1) 
                + (p7 == 0 && p8 == 1) + (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);

            int B = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
            int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
            int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
                image->ie[i][j].a |= 2;
        }
    }

    image_foreach(image, i, j) {
        int marker = image->ie[i][j].a >> 1;
        int old = image->ie[i][j].a & 1;
        image->ie[i][j].a = old & (!marker);

        if (image->ie[i][j].a != old)
            need_check_more = 1;
    }

    return need_check_more;
};

// https://github.com/LingDong-/skeleton-tracing
int image_skeleton(IMAGE* image)
{
    BYTE n;
    int i, j;

    check_image(image);

    // Save bitmap to channel A ...
    image_foreach(image, i, j) {
        color_rgb2gray(image->ie[i][j].r, image->ie[i][j].g, image->ie[i][j].b, &n);
        image->ie[i][j].a = (n > 128)? 1 : 0;
    }

    int need_check_more = 1;
    do {
        // Stage 1
        need_check_more &= __thinning_step(image, 0);
        // Stage 2
        need_check_more &= __thinning_step(image, 1);
    } while (need_check_more);

    // Save results ...
    image_foreach(image, i, j) {
        image->ie[i][j].a = (image->ie[i][j].a > 0)? 255 : 0;
    }

    return RET_OK;
}

// Middle value edge detection
int shape_midedge(IMAGE* img)
{
    int i, j, mid_thr;
    MATRIX* mat;

    check_image(img);
    mat = matrix_create(img->height, img->width);
    check_matrix(mat);
    mid_thr = color_midval(img, 'A');

    image_foreach(img, i, j)
    {
        mat->me[i][j] = 0;
        if (i == 0 || i == img->height - 1 || j == 0 || j == img->width - 1) // skip border points
            continue;

        if (__higher_cross('A', img, i, j, mid_thr))
            mat->me[i][j] = 1;
    }
    image_foreach(img, i, j)
    {
        if ((int)mat->me[i][j])
            img->ie[i][j].r = img->ie[i][j].g = img->ie[i][j].b = 255;
        else
            img->ie[i][j].r = img->ie[i][j].g = img->ie[i][j].b = 0;
    }
    matrix_destroy(mat);

    return RET_OK;
}

int shape_bestedge(IMAGE* img, float lr, float hr)
{
    char* p;
    int i, j, ret;

    check_image(img);
    image_foreach(img, i, j) img->ie[i][j].r = image_getvalue(img, 'A', i, j);

    img->format = IMAGE_GRAY;
    image_gauss_filter(img, 2.0);

    p = getenv("CANNY_EDGE_LO_THRESHOLD");
    lr = (p) ? atof(p) : lr;
    p = getenv("CANNY_EDGE_HI_THRESHOLD");
    hr = (p) ? atof(p) : hr;
    ret = __canny_edge_detect(img, hr, lr);
    image_foreach(img, i, j)
    {
        if (img->ie[i][j].b == 255)
            img->ie[i][j].r = img->ie[i][j].g = 255;
        else
            img->ie[i][j].r = img->ie[i][j].g = img->ie[i][j].b = 0;
    }

    return ret;
}


IMAGE *image_get_connected(IMAGE *image)
{
    int i, j, r, c, k, nr, nc;
    static int inbs[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
    static int jnbs[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
    DOTS *ds = dot_set();

    CHECK_IMAGE(image);

    image_foreach(image, i, j) {
        if (image->ie[i][j].a == 0)
            continue;

        IMAGE *outimg = image_copy(image);
        CHECK_IMAGE(outimg);

        image_foreach(outimg, r, c) {
            outimg->ie[r][c].a = 0;
        }

        // Start trace ...
        ds->count = 0;
        dot_put(i, j); // push stack
        while(ds->count > 0) {
            // pop stack
            ds->count--; 
            r = ds->dot[ds->count].r;
            c = ds->dot[ds->count].c;

            // output r, c
            outimg->ie[r][c].a = 255;
            image->ie[r][c].a = 0; // clear avoid loop check

            // Check nb of (r, c)
            for (k = 0; k < 8; k++) {
                nr = r + inbs[k];
                nc = c + jnbs[k];

                if (image->ie[nr][nc].a > 0)
                    dot_put(nr, nc);
            }
        }

        // return for next
        return outimg; 
    }

    return NULL; // not found connected ...
}

int image_parse_arrow(IMAGE *image, int *start_row, int *start_col, int *stop_row, int *stop_col)
{
    RECT rect;
    int weight[4]; // top_left, top_right, bottom_left, bottom_righ;
    int i, j, r1, c1, r2, c2;

    check_image(image);
    r1 = image->height; c1 = image->width;
    r2 = 0; c2 = 0;
    image_foreach(image, i, j) {
        if (image->ie[i][j].a == 0)
            continue;

        r1 = MIN(i, r1); c1 = MIN(j, c1);
        r2 = MAX(i, r2); c2 = MAX(j, c2);
    }
    rect.r = r1; rect.h = r2 - r1;
    rect.c = c1; rect.w = c2 - c1;

    /*************************************************************
     * Weight layout 
     *  0 | 1
     *  2 | 3
    *************************************************************/
    weight[0] = weight[1] = weight[2] = weight[3] = 0;
    rect_foreach(&rect, i, j)
    {
        if (image->ie[rect.r + i][rect.c + j].r == 0)
            continue;
        if (i < rect.h/2) { //top
            if (j < rect.w/2) {
                weight[0]++;
            } else {
                weight[1]++;
            }
        } else { // bottom
            if (j < rect.w/2)
                weight[2]++;
            else
                weight[3]++;
        }
    }

    *start_row = rect.r + rect.h/2;
    *start_col = rect.c + rect.w/2;

    if (weight[0] >= weight[1] && weight[0] >= weight[2] && weight[0] >= weight[3]) {
        // weight[0] 
        *stop_row = rect.r;
        *stop_col = rect.c;
    }
    if (weight[1] >= weight[0] && weight[1] >= weight[2] && weight[1] >= weight[3]) {
        // weight[1] 
        *stop_row = rect.r;
        *stop_col = rect.c + rect.w;
    }
    if (weight[2] >= weight[0] && weight[2] >= weight[1] && weight[2] >= weight[3]) {
        // weight[2] 
        *stop_row = rect.r + rect.h;
        *stop_col = rect.c;
    }
    if (weight[3] >= weight[0] && weight[3] >= weight[1] && weight[3] >= weight[2]) {
        // weight[3] 
        *stop_row = rect.r + rect.h;
        *stop_col = rect.c + rect.w;
    }

    // extend vector from center 
    *start_row = *start_row - (*stop_row - *start_row);
    *start_col = *start_col - (*stop_col - *start_col);

    return RET_OK;
}
