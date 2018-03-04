#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

#include <png.h>

#include "render-utils.h"

void
pixel_set (double *framebuffer,
           int x,
           int y,
           int z,
           double red,
           double green,
           double blue)
{
  if (x < 0 || y < 0 || z < 0)
    return;

  if (x >= 8 || y >= 8 || z >= 8)
    return;

  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 0] = red;
  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 1] = green;
  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 2] = blue;
}


void
render_pixel (double *framebuffer,
              int x,
              int y,
              int z,
              double red,
              double green,
              double blue,
              double alpha)
{
  if (x < 0 || y < 0 || z < 0)
    return;

  if (x >= 8 || y >= 8 || z >= 8)
    return;

  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 0] *= 1.0   - alpha;
  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 0] += red   * alpha;

  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 1] *= 1.0   - alpha;
  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 1] += green * alpha;

  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 2] *= 1.0   - alpha;
  framebuffer[(((x * 8) + y) * 8 + z) * 3 + 2] += blue  * alpha;
}


void
interpolate_pixel (double *fb,
                   double x,
                   double y,
                   double z,
                   double red,
                   double green,
                   double blue,
                   double alpha)
{
  render_pixel (fb,
                (int) x,
                (int) y,
                (int) z,
                red, green, blue,
                alpha *
                (1.0 - (x - (int) x)) *
                (1.0 - (y - (int) y)) *
                (1.0 - (z - (int) z)));

  render_pixel (fb,
                1 + (int) x,
                (int) y,
                (int) z,
                red, green, blue,
                alpha *
                ((x - (int) x)) *
                (1.0 - (y - (int) y)) *
                (1.0 - (z - (int) z)));

  render_pixel (fb,
                (int) x,
                1 + (int) y,
                (int) z,
                red, green, blue,
                alpha *
                (1.0 - (x - (int) x)) *
                ((y - (int) y)) *
                (1.0 - (z - (int) z)));

  render_pixel (fb,
                (int) x,
                (int) y,
                1 + (int) z,
                red, green, blue,
                alpha *
                (1.0 - (x - (int) x)) *
                (1.0 - (y - (int) y)) *
                ((z - (int) z)));

  render_pixel (fb,
                (int) x,
                1 + (int) y,
                1 + (int) z,
                red, green, blue,
                alpha *
                (1.0 - (x - (int) x)) *
                ((y - (int) y)) *
                ((z - (int) z)));

  render_pixel (fb,
                1 + (int) x,
                (int) y,
                1 + (int) z,
                red, green, blue,
                alpha *
                ((x - (int) x)) *
                (1.0 - (y - (int) y)) *
                ((z - (int) z)));

  render_pixel (fb,
                1 + (int) x,
                1 + (int) y,
                (int) z,
                red, green, blue,
                alpha *
                ((x - (int) x)) *
                ((y - (int) y)) *
                (1.0 - (z - (int) z)));

  render_pixel (fb,
                1 + (int) x,
                1 + (int) y,
                1 + (int) z,
                red, green, blue,
                alpha *
                ((x - (int) x)) *
                ((y - (int) y)) *
                ((z - (int) z)));
}


void
render_blob (double *framebuffer,
             double cx, double cy, double cz,
             double red, double green, double blue,
             double r, double s)
{
  int X, Y, Z;
  double x, y, z;

  for (X = 0; X < 8; X++)
    {
      for (Y = 0; Y < 8; Y++)
        {
          for (Z = 0; Z < 8; Z++)
            {
              double d;

              x = X * 0.25;
              y = Y * 0.25;
              z = Z * 0.25;

              d = pow ((x - cx) * (x - cx) +
                       (y - cy) * (y - cy) +
                       (z - cz) * (z - cz), 0.5);

              d /= r;
              d = (d - 0.5) * s + 0.5;
              d = CLAMP (d, 0.0, 1.0);

              d = pow (d, s);

              render_pixel (framebuffer, X, Y, Z, red, green, blue, 1.0 - d);
            }
        }
    }
}


void
framebuffer_set (double *framebuffer,
                 double red,
                 double green,
                 double blue)
{
  int x, y, z;

  for (x = 0; x < 8; x++)
    {
      for (y = 0; y < 8; y++)
        {
          for (z = 0; z < 8; z++)
            {
              pixel_set (framebuffer, x, y, z, red, green, blue);
            }
        }
    }
}


void
framebuffer_dim (double *framebuffer,
                 double alpha)
{
  int i;

  for (i = 0; i < 8 * 8 * 8 * 3; i++)
    framebuffer[i] *= alpha;
}


void
framebuffer_merge (double *fb,
                   double *effect1,
                   double *effect2,
                   double alpha)
{
  int i;

  for (i = 0; i < 8 * 8 * 8 * 3; i++)
    {
      fb[i] = effect1[i] * (1.0 - alpha) + effect2[i] * alpha;
    }
}


double
euclid_3d (double x,
           double y,
           double z)
{
	return sqrt (x * x + y * y + z * z);
}


int
read_png_file (char    *file_name,
               int     *ret_width,
               int     *ret_height,
               int     *ret_rowstride,
               double **ret_pixels)
{
  png_byte color_type;
  png_byte bit_depth;

  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  int x, y, width, height, rowstride;
  uint8_t *pngpixels;
  double *pixels;

  uint8_t header[8];    // 8 is the maximum size that can be checked
  FILE *fp;

  /* open file and test for it being a png */
  fp = fopen (file_name, "rb");
  if (!fp)
    {
      fprintf (stderr, "File %s could not be opened for reading\n",
               file_name);
      return -ENOENT;
    }

  if (fread (header, 1, 8, fp) < 8 ||
      png_sig_cmp (header, 0, 8))
    {
      fprintf (stderr, "File %s is not recognized as a PNG file\n",
               file_name);
      fclose (fp);
      return -EINVAL;
    }

  /* initialize stuff */
  png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
    {
      fprintf (stderr, "png_create_read_struct failed\n");
      fclose (fp);
      return -ENOMEM;
    }

  info_ptr = png_create_info_struct (png_ptr);
  if (!info_ptr)
    {
      fprintf (stderr, "png_create_info_struct failed\n");
      fclose (fp);
      return -ENOMEM;
    }

  if (setjmp (png_jmpbuf (png_ptr)))
    {
      fprintf (stderr, "Error during init_io\n");
      fclose (fp);
      return -ENOMEM;
    }

  png_init_io (png_ptr, fp);
  png_set_sig_bytes (png_ptr, 8);

  png_read_info (png_ptr, info_ptr);

  width = png_get_image_width (png_ptr, info_ptr);
  height = png_get_image_height (png_ptr, info_ptr);
  color_type = png_get_color_type (png_ptr, info_ptr);
  bit_depth = png_get_bit_depth (png_ptr, info_ptr);

  if (color_type != PNG_COLOR_TYPE_RGB || bit_depth != 8)
    {
      fprintf (stderr, "wrong PNG type\n");
      fclose (fp);
      return -EINVAL;
    }

  png_set_interlace_handling (png_ptr);
  png_read_update_info (png_ptr, info_ptr);

  /* read file */
  if (setjmp (png_jmpbuf (png_ptr)))
    {
      fprintf (stderr, "Error during read_image\n");
      fclose (fp);
      return -EIO;
    }

  rowstride = png_get_rowbytes (png_ptr, info_ptr);
  pngpixels = malloc (rowstride * height);

  row_pointers = malloc (sizeof (png_bytep) * height);
  for (y = 0; y < height; y++)
    {
      row_pointers[y] = pngpixels + y * rowstride;
    }

  png_read_image (png_ptr, row_pointers);

  free (row_pointers);
  fclose (fp);

  pixels = malloc (sizeof (double) * width * height * 3);

  for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
        {
          // fprintf (stderr, "x: %d, y: %d\n", x, y);
          pixels[(y * width + x) * 3 + 0] =
              ((double) pngpixels[y * rowstride + x * 3 + 0]) / 255.0;
          pixels[(y * width + x) * 3 + 1] =
              ((double) pngpixels[y * rowstride + x * 3 + 1]) / 255.0;
          pixels[(y * width + x) * 3 + 2] =
              ((double) pngpixels[y * rowstride + x * 3 + 2]) / 255.0;
        }
    }

  free (pngpixels);

  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);

  *ret_width = width;
  *ret_height = height;
  *ret_rowstride = 3 * width;
  *ret_pixels = pixels;

  return 0;
}


void
sample_buffer (double  *buffer,
               int      width,
               int      height,
               int      rowstride,
               double   x,
               double   y,
               double  *ret_pixel)
{
  int x0, y0;
  double dx, dy;

  x0 = floor (x);
  y0 = floor (y);
  dx = x - x0;
  dy = y - y0;

  ret_pixel[0] = 0.0;
  ret_pixel[1] = 0.0;
  ret_pixel[2] = 0.0;

  if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height)
    {
      ret_pixel[0] += (1.0 - dx) * (1.0 - dy) * buffer[y0 * rowstride + x0 * 3 + 0];
      ret_pixel[1] += (1.0 - dx) * (1.0 - dy) * buffer[y0 * rowstride + x0 * 3 + 1];
      ret_pixel[2] += (1.0 - dx) * (1.0 - dy) * buffer[y0 * rowstride + x0 * 3 + 2];
    }

  x0 += 1;

  if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height)
    {
      ret_pixel[0] += dx * (1.0 - dy) * buffer[y0 * rowstride + x0 * 3 + 0];
      ret_pixel[1] += dx * (1.0 - dy) * buffer[y0 * rowstride + x0 * 3 + 1];
      ret_pixel[2] += dx * (1.0 - dy) * buffer[y0 * rowstride + x0 * 3 + 2];
    }

  x0 -= 1;
  y0 += 1;

  if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height)
    {
      ret_pixel[0] += (1.0 - dx) * dy * buffer[y0 * rowstride + x0 * 3 + 0];
      ret_pixel[1] += (1.0 - dx) * dy * buffer[y0 * rowstride + x0 * 3 + 1];
      ret_pixel[2] += (1.0 - dx) * dy * buffer[y0 * rowstride + x0 * 3 + 2];
    }

  x0 += 1;

  if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height)
    {
      ret_pixel[0] += dx * dy * buffer[y0 * rowstride + x0 * 3 + 0];
      ret_pixel[1] += dx * dy * buffer[y0 * rowstride + x0 * 3 + 1];
      ret_pixel[2] += dx * dy * buffer[y0 * rowstride + x0 * 3 + 2];
    }
}

