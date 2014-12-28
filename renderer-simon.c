#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <netdb.h>
#include <math.h>
#include <sys/time.h>

#include "opc-client.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define CLAMP(v, lo, hi) MAX (MIN ((v), (hi)), (lo))

#define EFFECT_TIME 30.0


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
render_wave (double *framebuffer,
             double t)
{
  int X, Y;

  for (X = 0; X < 8; X++)
    {
      for (Y = 0; Y < 8; Y++)
        {
          double x, y, r, z;
          x = (((double) X) / 7.0 - 0.5) * 0.8 *  M_PI;
          y = (((double) Y) / 7.0 - 0.5) * 0.8 * M_PI;
          r = pow (x * x + y * y, 0.5);
          z = sin (r + t) * 0.7 + 0.7;

          render_pixel (framebuffer, X, Y, 1 + (int) z,
                        1.0, 0.0, 0.0, z - (int) z);
          render_pixel (framebuffer, X, Y, (int) z,
                        1.0, 0.0, 0.0, 1.0 - (z - (int) z));
        }
    }
}


void
mode_lava_balloon (double *fb,
                   double  t)
{
  framebuffer_set (fb, 0.0, 0.0, 0.4);
  render_wave (fb, t);
  render_blob (fb,
               0.875, 0.875, fmod (t, 4.0) - 1.0,
               1.0, 1.0, 0.0,
               0.75, 1.0);
}


void
mode_random_blips (double *fb,
                   double t)
{
  int x, y, z, i;
  framebuffer_dim (fb, 0.99);

  for (i = 0; i < 5; i++)
    {
      x = random () % 8;
      y = random () % 8;
      z = random () % 8;

      pixel_set (fb, x, y, z,
                 drand48 (), drand48 (), drand48 ());
    }
}


int
main (int   argc,
      char *argv[])
{
  double *framebuffer;
  double *effect1, *effect2;
  OpcClient *client;
  struct timeval tv;

  framebuffer = calloc (8 * 8 * 8 * 3, sizeof (double));
  effect1 = calloc (8 * 8 * 8 * 3, sizeof (double));
  effect2 = calloc (8 * 8 * 8 * 3, sizeof (double));

  client = opc_client_new ("localhost:7890", 7890,
                           8 * 8 * 8 * 3,
                           framebuffer);

  opc_client_connect (client);

  while (1)
    {
      double t, dt, alpha;
      gettimeofday (&tv, NULL);
      t = tv.tv_sec * 1.0 + tv.tv_usec / 1000000.0;

      mode_lava_balloon (effect1, t);
      mode_random_blips (effect2, t);

      dt = fmod (t, 2 * EFFECT_TIME);

      if (dt < 1.0)
        alpha = dt;
      else if (dt < EFFECT_TIME)
        alpha = 1.0;
      else if (dt < EFFECT_TIME + 1.0)
        alpha = EFFECT_TIME + 1.0 - dt;
      else
        alpha = 0.0;

      framebuffer_merge (framebuffer, effect1, effect2, alpha);

      opc_client_write (client, 0, 0);
      usleep (50 * 1000);  /* 50ms */
    }

  opc_client_shutdown (client);

  return 0;

}

