#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "opc-client.h"
#include "render-utils.h"

#include "renderer_astern.h"
#include "renderer_ball.h"

#define EFFECT_TIME 30.0

typedef void (*RenderFunc) (double *, double);


void
mode_import_png (double *fb,
                 double  t)
{
  double *pixels, *cfp;
  int width, height, rowstride;
  int x, y;
  double x1, y1, x2, y2, angle;

  if (read_png_file ("swirl.png",
                     &width, &height, &rowstride, &pixels) < 0)
    {
      fprintf (stderr, "failed to read PNG\n");
      return;
    }

  if (width < 32 || height < 31)
    {
      fprintf (stderr, "PNG not big enough\n");
      return;
    }

  for (y = 0; y < 16; y++)
    {
      for (x = 0; x < 32; x++)
        {
          cfp = fb + (y * 32 + x) * 3;

#if 0
          x1 = x * 0.5;
          y1 = y + (x % 2) * 0.5;

          x1 -= 16;
          y1 -= 16;

          y1 += 3;

          angle = fmod (t / 16, 2 * M_PI);
          x2 = cos (angle) * x1 - sin (angle) * y1;
          y2 = sin (angle) * x1 + cos (angle) * y1;
          x1 = x2;
          y1 = y2;

          x1 += fmod (t, 3 * EFFECT_TIME) * (width + 32) / (3 * EFFECT_TIME);
          x1 -= 32;

          x1 += 16;
          y1 += 16;
#else
          x1 = 15 + (x + 0) / 2 - y;
          y1 = 30 - (x + 1) / 2 - y;
#endif

          sample_buffer (pixels, width, height, rowstride, x1, y1, cfp);
        }
    }

  free (pixels);
}


void
mode_radar_scan (double *fb,
                 double  t)
{
  int i;

  for (i = 0; i < 512; i++)
    {
      double x, y, phi, r, alpha;

      x = (i % 32) / 2.0           - 7.75;
      y = (i / 32) + (i % 2) * 0.5 - 7.75;

      phi = fmod (M_PI * 2 + atan2 (y, x) + t, M_PI * 2);
      r = hypot (x, y);

      if (r < 8.5)
        {
          alpha = r < 7.5 ? 1.0 : 8.5 - r;
          phi = MAX (2.0 - phi, 0.0) / 2.0;
          fb[i*3 + 0] = 1.0 * phi * alpha;;
          fb[i*3 + 1] = (0.4 + 0.6 * phi) * alpha;
          fb[i*3 + 2] = 1.0 * phi * alpha;
        }
      else
        {
          fb[i*3 + 0] = 0;
          fb[i*3 + 1] = 0;
          fb[i*3 + 2] = 0;
        }
    }
}


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
          offsets[i] = drand48 () * 7 - 3.5;
        }
    }

  framebuffer_set (fb, 0.0, 0.3, 0.0);

  for (x = 0; x < 8; x++)
    {
      for (y = 0; y < 8; y++)
        {
          double z;

          z = CLAMP (sin (t) * 7 + offsets[x * 8 + y] + 3.5 , 0.0, 6.99);

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
                   double  t)
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


void
mode_astern (double *framebuffer,
             double  t)
{
  static int wait_counter = -1;    // wait when finished
  static int finished_astern = 0;  //
  static int initiated_astern = 0;

  if (!initiated_astern)
    {
      initiated_astern = 1;
      init_astern ();
    }

  if (!finished_astern)
    {
      render_map (framebuffer);
      finished_astern = astern_step ();
      if (finished_astern < 0)
        {
          // fail, no route found

          wait_counter = 0;
          framebuffer_set (framebuffer, 1.0, 1.0, 1.0);
        }
     }
   else
     {
       if (wait_counter < 0)
         wait_counter = 9;

       if (wait_counter > 0)
         {
           render_path (framebuffer);
           wait_counter--;
         }
       else
         {
           // == 0
           destruct_astern ();
           initiated_astern = 0;
           finished_astern = 0;
           wait_counter = -1;
         }
     }
}

void
mode_ball_wave (double *fb,
                double  t)
{
  render_ball (t,fb);
}


void
mode_rect_flip (double *fb,
                double  t)
{
  int x, y, z;
  double dt, sdt, cdt;
  double nx, ny, nz, a;
  int pos;

  pos = (int) (fmod (t, 6.0 * 2.0) / 2.0);
  dt  = fmod (t, 2.0) / 2.0;

  dt = pow (dt, 3);

  dt = dt * M_PI / 2;
  cdt = cos (dt);
  sdt = sin (dt);

  switch (pos)
    {
      case 0:
        nx = + sdt;
        ny = 0;
        nz = - cdt;
        a = 7.0 * (nx + nz);
        break;
      case 1:
        nx = + cdt;
        ny = + sdt;
        nz = 0;
        a = 7.0 * nx;
        break;
      case 2:
        nx = 0;
        ny = + cdt;
        nz = - sdt;
        a = 0.0;
        break;
      case 3:
        nx = - sdt;
        ny = 0;
        nz = + cdt;
        a = 0.0;
        break;
      case 4:
        nx = - cdt;
        ny = - sdt;
        nz = 0;
        a = 7.0 * ny;
        break;
      case 5:
        nx = 0;
        ny = - cdt;
        nz = + sdt;
        a = 7.0 * (ny + nz);
        break;
      default:
        break;
    }

  framebuffer_set (fb, 0.2, 0.0, 0.0);

  for (x = 0; x < 7.99; x++)
    {
      for (y = 0; y < 7.99; y++)
        {
          for (z = 0; z < 7.99; z++)
            {
              double d, len;

              d = fabs (nx * x + ny * y + nz * z - a);

              switch (pos)
                {
                  case 0:
                    len = sqrt ((7.0 - x) * (7.0 - x) + (7.0 - z) * (7.0 - z));
                    break;
                  case 1:
                    len = sqrt ((7.0 - x) * (7.0 - x) + y * y);
                    break;
                  case 2:
                    len = sqrt (y * y + z * z);
                    break;
                  case 3:
                    len = sqrt (x * x + z * z);
                    break;
                  case 4:
                    len = sqrt (x * x + (7.0 - y) * (7.0 - y));
                    break;
                  case 5:
                    len = sqrt ((7.0 - y) * (7.0 - y) + (7.0 - z) * (7.0 - z));
                    break;
                  default:
                    len = 0;
                    break;
                }

              if (d < 0.7)
                {
                  len = 1.0 - CLAMP (len - 7.0, 0.0, 1.0);
                  render_pixel (fb, x, y, z, 1.0, 0.8, 0.0, len * (1.0 - d));
                }
            }
        }
    }
}


int
main (int   argc,
      char *argv[])
{
  double *framebuffer;
  double *effect1, *effect2;
  struct timeval tv;
  OpcClient *client;
  int mode = 0;
  int have_flip = 0;
  RenderFunc modeptrs[] =
    {
      // mode_astern,
      mode_lava_balloon,
      mode_jumping_pixels,
      mode_import_png,
      mode_random_blips,
      mode_rect_flip,
      mode_ball_wave,
      mode_radar_scan,
    };

  int num_modes = sizeof (modeptrs) / sizeof (modeptrs[0]);

  framebuffer = calloc (8 * 8 * 8 * 3, sizeof (double));
  effect1 = calloc (8 * 8 * 8 * 3, sizeof (double));
  effect2 = calloc (8 * 8 * 8 * 3, sizeof (double));

  client = opc_client_new (argc > 1 ? argv[1] : "127.0.0.1:7890", 15163,
                           // "balldachin.hasi:7890", 7890,
                           8 * 8 * 8 * 3,
                           framebuffer);

  if (!client)
    {
      fprintf (stderr, "can't open client\n");
      exit (1);
    }
  opc_client_connect (client);

  if (argc > 2)
    {
      int mode = atoi (argv[2]);

      if (mode < num_modes)
        {
          modeptrs[0] = modeptrs[mode];
          num_modes = 1;
        }
    }

  while (1)
    {
      double t, dt;
      gettimeofday (&tv, NULL);
      t = tv.tv_sec * 1.0 + tv.tv_usec / 1000000.0;

      dt = fmod (t, EFFECT_TIME);

      if (dt < 1.0)
        {
          if (have_flip == 1)
            {
              memset (effect2, 0, sizeof (double) * 8 * 8 * 8 * 3);
              have_flip = 0;
            }

          modeptrs[(mode + 0) % num_modes] (effect1, t);
          modeptrs[(mode + 1) % num_modes] (effect2, t);

          framebuffer_merge (framebuffer, effect1, effect2, dt);
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

