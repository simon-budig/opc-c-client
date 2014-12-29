#define NUM (8*8*8)
#define SIZE 8

typedef enum {Unseen, Open, Closed, Wall} State;

typedef struct Node {
	State state;
	double d; // distance to dest
	double pc; //
	int x, y, z;
	struct Node* route_from;
	} Node_t;

  
void init_astern(); 
void destruct_astern(); 
int astern_step();
void render_map(double* fb);
void render_path(double* fb);
