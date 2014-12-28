
#include "frame.c"

double euclid_3d(double x, double y, double z) 
{
	return sqrt(pow(x,2) + pow(y,2) + pow(z,2));
}

double ramp(double cycle, double low, double high, double t) {
	double c = fmod(t,cycle);
	if(c < low || c >= high) 
		return 0.0;
	else
		return (c-low)/(high-low);

}
double inv_ramp(double cycle, double low, double high, double t) {
	double r = ramp(cycle, low, high, t);
	if (r > 0.0)
		r = 1.0-r;
	return r;
}

void render_ball(double t,
	double* fb)
{
  int x, y, z;
  double cx=3.5, cy=3.5, cz=3.5;

  for (x = 0; x < 8; x++)
    {
      for (y = 0; y < 8; y++)
        {
          for (z = 0; z < 8; z++)
            {
	      double d = euclid_3d(x, y, z);
	      double td = t+d; // time distance from centre
	      double red = ramp(7,0,1*2,td) + inv_ramp(3,2*2,3*2,td);
	      double green = ramp(7,1,2*2,td) + inv_ramp(3,0,1*2,td);
	      double blue = ramp(7,2*2,3*2,td) + inv_ramp(3,1*2,2*2,td);
              pixel_set (fb, x, y, z, red, green, blue);
            }
        }
    }
			
}

	

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

