#include <math.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define CLAMP(v, lo, hi) MAX (MIN ((v), (hi)), (lo))


void pixel_set         (double *framebuffer,
                        int x,
                        int y,
                        int z,
                        double red,
                        double green,
                        double blue);

void render_pixel      (double *framebuffer,
                        int x, int y, int z,
                        double red, double green, double blue,
                        double alpha);

void interpolate_pixel (double *framebuffer,
                        double x, double y, double z,
                        double red, double green, double blue,
                        double alpha);

void render_blob       (double *framebuffer,
                        double cx, double cy, double cz,
                        double red, double green, double blue,
                        double r, double s);

void framebuffer_set   (double *framebuffer,
                        double  red,
                        double  green,
                        double  blue);

void framebuffer_dim   (double *framebuffer,
                        double  alpha);

void framebuffer_merge (double *fb,
                        double *effect1,
                        double *effect2,
                        double  alpha);

double euclid_3d       (double x,
                        double y,
                        double z);

int read_png_file      (char    *file_name,
                        int     *width,
                        int     *height,
                        int     *rowstride,
                        double **pixels);

void sample_buffer     (double  *buffer,
                        int      width,
                        int      height,
                        int      rowstride,
                        double   x,
                        double   y,
                        double  *ret_pixel);

