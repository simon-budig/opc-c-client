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

#define EFFECT_TIME 30.0


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

