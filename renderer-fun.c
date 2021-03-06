
#include "opc-client.h"
#include "render-utils.h"
#include "renderer_ball.h"

int
main (int   argc,
      char *argv[])
{
  double *framebuffer;
  OpcClient *client;
  struct timeval tv;

  framebuffer = calloc (8 * 8 * 8 * 3, sizeof (double));

  client = opc_client_new ("localhost:7890", 7890,
                           8 * 8 * 8 * 3,
                           framebuffer);

  opc_client_connect (client);

  while (1)
    {
      double t;
      gettimeofday (&tv, NULL);
      t = tv.tv_sec * 1.0 + tv.tv_usec / 1000000.0;

//      render_wave (t, framebuffer);
      // framebuffer_set (framebuffer, 0.0, 0.0, 0.3);
 /*     render_blob (framebuffer,
                   0.875, 0.875, fmod (t, 4.0) - 1.0,
                   1.0, 1.0, 0.0,
                   0.75, 1.0);
 */
 	render_ball(t, framebuffer);

      opc_client_write (client, 0, 0);
      usleep (50 * 1000);  /* 50ms */
    }

  opc_client_shutdown (client);

  return 0;

}

