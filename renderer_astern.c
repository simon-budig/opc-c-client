#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "opc-client.h"
#include "render-utils.h"

#include "renderer_astern.h"

struct Node** set;

struct Node dest = { Unseen, 0.0,0.0, 0,0,0, 0};
struct Node start = { Open, -1.0,-1.0, 7,7,7, 0};


double cost_est(const Node_t* const n){
	if (n->pc >=0)
		return n->d + n->pc;
	else 
		return n->d;
}

void cpy_node_vals(const Node_t* const src, Node_t* const dst) {
    dst->state = src->state;
    dst->x = src->x;
    dst->y = src->y;
    dst->z = src->z;
    dst->d = src->d;
    dst->pc = src->pc;
    dst->route_from = src->route_from;
}

int node_at_pos(const Node_t* const a, const double x, const double y, const double z) {
	return a->x == x && a->y == y && a->z == z;
}

int node_same_pos(const Node_t* const a, const Node_t* const b) {
	return node_at_pos(a, b->x, b->y, b->z);
}


int node_is_neigh(struct Node* a, struct Node* neigh) {
   return 1 == abs(a->x - neigh->x) + abs(a->y - neigh->y) + abs(a->z - neigh->z);
}


Node_t* dst_node() {
 int i;
 for(i=0; i < NUM; i++) {
    if (node_same_pos(set[i], &dest))
	    return set[i];
 }
 return 0;
}

void print_dists() 
  { int i;
  for (i=0; i<NUM; i++) 
	  printf("%f\n", set[i]->d);
  }

int cmp_dst(const void* aa, const void* bb) {
	Node_t* a;
	Node_t* b;
	a = *(Node_t**) aa;
	b = *(Node_t**) bb;

	if(cost_est(a) < cost_est(b)) 
		return -1;
	if(cost_est(a) > cost_est(b)) 
		return 1;
	return 0;
}

int cmp_est_cost(const void* a, const void*b) {
	int ca, cb;
	ca=cost_est(a);
	cb=cost_est(b);
	if (ca < cb) return 1;
	if (ca > cb) return -1;	
	return 0;
}

void destruct_astern() {
 int i;
 for (i = 0; i < NUM ; i++) {
	free(set[i]);	 
 	set[i] = 0;
 }
 free(set);
 set = 0;
}

void setRandomWall(int height, int width, char ortho) 
{
 int i, o_off, h_off, w_off;
 // select random offset on ortho axis 
 // select offset of height
 // select offset of width
 o_off = (lrand48()+SIZE) % SIZE;
 h_off = (lrand48()+SIZE-height) % (SIZE - height);
 w_off = (lrand48()+SIZE-width) % (SIZE - width);

 if(ortho == 'r') 
	 ortho = lrand48() % 6;

 for (i=0; i<NUM ; i++) {
    Node_t* n = set[i];
    if( node_same_pos(n,&dest)
     || node_same_pos(n, &start)) continue;
    switch (ortho) {
    case 0:
    case 'x': if( n->x == o_off 
               && n->y > h_off
	       && n->y <= h_off + width 
	       && n->z > w_off 
	       && n->z <= w_off + width )
    		  n->state = Wall;
	break;
    case 1:
    case 'y':  if( n->y == o_off 
               && n->z > h_off
	       && n->z <= h_off + width 
	       && n->x > w_off 
	       && n->x <= w_off + width )
    		  n->state = Wall;
	break;
    case 2:
    case 'z':  if( n->z == o_off 
               && n->x > h_off
	       && n->x <= h_off + width 
	       && n->y > w_off 
	       && n->y <= w_off + width )
    		  n->state = Wall;
	break;
    case 3:
    case 'X': if( n->x == o_off 
               && n->z > h_off
	       && n->z <= h_off + width 
	       && n->y > w_off 
	       && n->y <= w_off + width )
    		  n->state = Wall;
	break;
    case 4:
    case 'Y':  if( n->y == o_off 
               && n->x > h_off
	       && n->x <= h_off + width 
	       && n->z > w_off 
	       && n->z <= w_off + width )
    		  n->state = Wall;
	break;
    case 5:
    case 'Z':  if( n->z == o_off 
               && n->y > h_off
	       && n->y <= h_off + width 
	       && n->x > w_off 
	       && n->x <= w_off + width )
    		  n->state = Wall;
        break;

    }
 }
	
}	

void init_astern() {
  double x,y,z;
  int i;
  start.d = euclid_3d(start.x - dest.x, start.y - dest.y, start.z - dest.z);
  start.pc = 0;
  set = malloc(sizeof(struct Node*) * NUM);
  for (x = 0; x < 8; x++)
    {
      for (y = 0; y < 8; y++)
        {
          for (z = 0; z < 8; z++)
            {
              struct Node* n = 0;
	      int i = 8*8*x + 8*y + z;
	      n = malloc(sizeof(struct Node));
	      if(node_at_pos(&dest, x,y,z)) // is destination
		  cpy_node_vals(&dest, n);
	      else if(node_at_pos(&start, x,y,z)) // is start 
		  cpy_node_vals(&start, n);
	      else {
 	     	  n->state = Unseen;
	     	  n->x = x;
	     	  n->y = y;
	     	  n->z = z;
	     	  n->d = euclid_3d(x-dest.x,y-dest.y,z-dest.z);
		  n->pc= euclid_3d(start.x-x, start.y-y, start.z-z);
		  n->route_from = 0;
	      }
	      set[i] = n;
            }
        }
    }
  // sort set by d
  //void qsort(void *base, size_t nmemb, size_t size,
  //                    int (*compar)(const void *, const void *));
//  print_dists();
//  printf("--------------------\n");
  qsort(&set[0], NUM, sizeof(Node_t*), cmp_est_cost);
  //print_dists();
  // Set Wall 1
/*  for (i=0; i<NUM ; i++) {
     Node_t* n = set[i];
//     if(n->x == 2 && n->z < 7 && n->y <= 6)
//	     n->state = Wall;
     if(n->x == 6 && n->z > 3 && n->y >= 0)
	     n->state = Wall;
  }
*/
  setRandomWall(7,7, 'x');
  setRandomWall(3,4, 'r');
  setRandomWall(2,2, 'r');
  setRandomWall(6,3, 'r');
  setRandomWall(4, 5, 'r');
}



int astern_step() {
// find closest open Node (n)
 Node_t* n = 0;
 Node_t* neigh[6] = {0};
 int i, found;
 for(i = 0; i < NUM && n == 0; i++) {
    if(set[i]->state == Open) {
	    n = set[i];
	    break;
    }
 }
if(!n) return -1; 
// if n is dest -> juchuu
 if(node_same_pos(n, &dest)) 
	 return 1;
// else set n Closed
 n->state = Closed;
// find Unseen neighbours of n (horrendously innefficient)
 for(i = 0, found=0; i < NUM && found < 7; i++) {
   if(node_is_neigh(n, set[i]) && (set[i]->state == Open || set[i]->state == Unseen)) {
	   neigh[found] = set[i];
// set neighbours to open
           neigh[found]->state = Open;
 	   neigh[found]->pc = n->pc + 1;
// set from to n
	   neigh[found]->route_from = n;
	   found++;
   }
 }
// sort for total cost
// done.
 qsort(set, NUM, sizeof(Node_t*), cmp_est_cost);
 return 0;
}






void render_map(double* fb)
{
  int i;
  for(i=0; i < NUM; i++) {
     double red=0.0, green=0.0, blue=0.0;  
     Node_t* n = set[i];
     if(node_same_pos(n, &start)) {
	 red=1.0; // Red
     }
     else if (node_same_pos(n, &dest)) {
         red=1.0; // Red
     }
     else {
	switch (n->state) {
	case Unseen: // Off
	   red = 0.1;
	   green = blue = 0.3;
	   break;
	case Wall: // Orange
	   red = 0.9;
           green = 0.9;
	   break;
	case Open: // Green
	   green = 1.0;
	   break;
	case Closed: // Blue
	   blue = 1.0;
	   break;
	}	  
     }
     pixel_set (fb, n->x, n->y, n->z, red, green, blue);
  }			
}

void render_path(double* fb) {
   Node_t* d;
   Node_t* n;
// find dst node
   d = dst_node();
   n = d->route_from;
// follow path backward, colour every node white
   for(;n != 0 && !node_same_pos(n,&start); n = n->route_from) {
	pixel_set(fb, n->x, n->y, n->z, 1.0, 1.0,1.0);
   }
}	


