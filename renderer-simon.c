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
#include "render-utils.h"

#define EFFECT_TIME 10.0

typedef void (*RenderFunc) (double *, double);


void
mode_jumping_pixels (double *fb,
                     double  t)
{
  static double *offsets = NULL;
  int i, x, y;

  if (!offsets)
    {
      offsets = malloc (64 * sizeof (double));
      for (i = 0; i < 64; i++)
        {
          offsets[i] = drand48 () * 7;
        }
    }

  framebuffer_set (fb, 0.0, 0.3, 0.0);

  for (x = 0; x < 8; x++)
    {
      for (y = 0; y < 8; y++)
        {
          double z;

          z = CLAMP (fmod (t * 6, 28.0) + offsets[x * 8 + y] - 14.0, 0.0, 6.99);

          render_pixel (fb, x, y, 1 + (int) z,
                        1.0, 1.0, 1.0, z - (int) z);
          render_pixel (fb, x, y, (int) z,
                        1.0, 1.0, 1.0, 1.0 - (z - (int) z));
        }
    }
}


void
mode_lava_balloon (double *fb,
                   double  t)
{
  int X, Y;

  framebuffer_set (fb, 0.0, 0.0, 0.4);

  for (X = 0; X < 8; X++)
    {
      for (Y = 0; Y < 8; Y++)
        {
          double x, y, r, z;
          x = (((double) X) / 7.0 - 0.5) * 0.8 *  M_PI;
          y = (((double) Y) / 7.0 - 0.5) * 0.8 * M_PI;
          r = pow (x * x + y * y, 0.5);
          z = sin (r + t) * 0.7 + 0.7;

          render_pixel (fb, X, Y, 1 + (int) z,
                        1.0, 0.0, 0.0, z - (int) z);
          render_pixel (fb, X, Y, (int) z,
                        1.0, 0.0, 0.0, 1.0 - (z - (int) z));
        }
    }

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
  int mode = 0;
  int have_flip = 0;
  RenderFunc modeptrs[] =
    {
      mode_lava_balloon,
      mode_jumping_pixels,
      mode_random_blips,
    };

  int num_modes = sizeof (modeptrs) / sizeof (modeptrs[0]);

  framebuffer = calloc (8 * 8 * 8 * 3, sizeof (double));
  effect1 = calloc (8 * 8 * 8 * 3, sizeof (double));
  effect2 = calloc (8 * 8 * 8 * 3, sizeof (double));

  client = opc_client_new ("localhost:7890", 7890,
                           8 * 8 * 8 * 3,
                           framebuffer);

  opc_client_connect (client);

  while (1)
    {
      double t, dt;
      gettimeofday (&tv, NULL);
      t = tv.tv_sec * 1.0 + tv.tv_usec / 1000000.0;

      dt = fmod (t, EFFECT_TIME);

      if (dt < 1.0)
        {
          modeptrs[(mode + 0) % num_modes] (effect1, t);
          modeptrs[(mode + 1) % num_modes] (effect2, t);

          framebuffer_merge (framebuffer, effect1, effect2, dt);
          have_flip = 0;
        }
      else
        {
          if (have_flip == 0)
            {
              double *tmp;

              tmp = effect1;
              effect1 = effect2;
              effect2 = tmp;
              mode += 1;
              mode %= num_modes;

              have_flip = 1;
            }

          modeptrs[(mode + 0) % num_modes] (effect1, t);
          framebuffer_merge (framebuffer, effect1, effect2, 0.0);
        }

      opc_client_write (client, 0, 0);
      usleep (50 * 1000);  /* 50ms */
    }

  opc_client_shutdown (client);

  return 0;
}

