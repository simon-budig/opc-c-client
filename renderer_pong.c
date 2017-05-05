#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "opc-client.h"
#include "render-utils.h"

#include "renderer_pong.h"

#define PADDLE_SIZE 0.65

static int have_init = 0;
static double last_t = 0;
static double px, py, pz;
static double dx, dy, dz;
static double g = -9.81 / 2;
static int state;

void
render_paddle (double *framebuffer,
               double  x,
               double  y,
               double  z,
               double  red,
               double  green,
               double  blue,
               double  size)
{
  int ix, iy;

  x *= 4;
  y *= 4;
  size *= 4;

  ix = (int) (x - size/2);
  iy = (int) (y - size/2);

  for (iy = (int) (y - size/2); iy < y + size/2 + 1; iy ++)
    {
      for (ix = (int) (x - size/2); ix < x + size/2 + 1; ix ++)
        {
          double alpha =
              (CLAMP (size - ABS (((double) ix) - x), 0.0, 1.0) *
               CLAMP (size - ABS (((double) iy) - y), 0.0, 1.0));

          render_pixel (framebuffer, ix, iy, ROUND (z),
                        red, green, blue, alpha);
        }
    }
}


void render_pong (double  t,
                  double* fb,
                  double  joy_x,
                  double  joy_y)
{
  double dt;

  joy_x = ( joy_x + 1.0) / 2.0 * (1.75 - PADDLE_SIZE) + PADDLE_SIZE / 2;
  joy_y = (-joy_y + 1.0) / 2.0 * (1.75 - PADDLE_SIZE) + PADDLE_SIZE / 2;

  if (!have_init || pz < -10.0)
    {
      have_init = 1;
      dt = 0.0;
      px = drand48 () * 1.75;
      py = drand48 () * 1.75;
      pz = 2.0;
      dx = dy = dz = 0.0;
      last_t = t;
      state = 0;
    }
  else
    {
      dt = t - last_t;
      last_t = t;

      dz += g * dt;

      px += dx * dt;
      py += dy * dt;
      pz += dz * dt;

      if (px < 0.0)
        {
          px = 0.0 - px;
          dx *= -1.0;
        }
      else if (px > 1.75)
        {
          px = 1.75 - px + 1.75;
          dx *= -1.0;
        }

      if (py < 0.0)
        {
          py = 0.0 - py;
          dy *= -1.0;
        }
      else if (py > 1.75)
        {
          py = 1.75 - py + 1.75;
          dy *= -1.0;
        }

      if (state == 0 && pz < 0.0)
        {
          double hit_x, hit_y;

          hit_x = joy_x - px;
          hit_y = joy_y - py;

          fprintf (stderr, "%.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n", px, py, joy_x, joy_y, hit_x, hit_y);
          if (ABS (hit_x) > PADDLE_SIZE / 2.0 ||
              ABS (hit_y) > PADDLE_SIZE / 2.0)
            {
              state = 2;
            }
          else
            {
              state = 1;
            }

          if (state == 1)
            {
              pz = 0.0 - pz;
              dz *= -1.0;
              dx += drand48() * hit_x;
              dy += drand48() * hit_y;
            }
        }
    }

  framebuffer_set (fb, 0.05, 0.0, 0.25);

  render_blob (fb, px, py, pz, 0.0, 1.0, 1.0, 0.7, 1.5);
  if (state == 1)
    render_paddle (fb, joy_x, joy_y, 0, 0.0, 1.0, 0.0, 7);
  else if (state == 2 || pz < 0.0)
    render_paddle (fb, joy_x, joy_y, 0, 1.0, 0.3, 0.0, 7);

  render_paddle (fb, joy_x, joy_y, 0, 1.0, 1.0, 0.0, PADDLE_SIZE);

  if (state == 1)
    state = 0;
}


