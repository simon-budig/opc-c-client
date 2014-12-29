#include "opc-client.h"
#include "render-utils.h"
#include "renderer_astern.h"

int
main (int   argc,
      char *argv[])
{
  double *framebuffer;
  OpcClient *client;
  struct timeval tv;
  int finished = 0;


  framebuffer = calloc (8 * 8 * 8 * 3, sizeof (double));

  client = opc_client_new ("localhost:7890", 7890,
                           8 * 8 * 8 * 3,
                           framebuffer);

  opc_client_connect (client);

  while(1) {
  framebuffer_set(framebuffer, 0.0,0.0,0.0);
  init();
  finished = 0;

  while (!finished)
    {
      
      double t;
      gettimeofday (&tv, NULL);
      t = tv.tv_sec * 1.0 + tv.tv_usec / 1000000.0;

 	render_map(framebuffer);
	finished = astern_step();

      opc_client_write (client, 0, 0);
      usleep (100 * 1000);  /* 50ms */
    }
   
  render_path(framebuffer);
  opc_client_write (client, 0, 0);
  sleep(2);
  destruct();
  }
  opc_client_shutdown (client);

  return 0;

}

