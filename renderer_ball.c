#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "opc-client.h"
#include "render-utils.h"

#include "renderer_ball.h"

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


double triangle_ramp(double peak, double radius, double var)
{
   double r = fmod( var + radius - peak + 5.83, 5.83 ) / radius;
   if(r > 1.0) 
	   r = 2.0 - r;
   if(r < 0.0) 
	   r = 0.0;
//   printf("%f\n", r);
   return r;
}

void render_ball(double t,
	double* fb)
{
  int x, y, z;
  double ta = fmod(t, 2*3.1415); // time angle in radians	
  double cx=3.5, cy=3.5, cz=3.5;
  double ar = 3;	
  cx += ar*cos(ta);
  cy += ar*sin(ta);
  cz += ar*cos(fmod(ta+3.14, 2*3.1415));
  

  for (x = 0; x < 8; x++)
    {
      for (y = 0; y < 8; y++)
        {
          for (z = 0; z < 8; z++)
            {
	      double d = euclid_3d(x - cx, y - cy, z - cz);
	      double td = fmod(t-d, 5.83); // time distance from centre
	      double red = triangle_ramp(2.33, 2.33, td);
	      //ramp(7,0,1*2,td) + inv_ramp(3,2*2,3*2,td);
	      double green = triangle_ramp(4.66, 2.33, td);
	      //ramp(7,1,2*2,td) + inv_ramp(3,0,1*2,td);
	      double blue = triangle_ramp(0, 2.33, td);
	      //ramp(7,2*2,3*2,td) + inv_ramp(3,1*2,2*2,td);
              pixel_set (fb, x, y, z, red, green, blue);
            }
        }
    }
			
}

	
