#ifndef __OPC_CLIENT_H__
#define __OPC_CLIENT_H__

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define CLAMP(v, lo, hi) MAX (MIN ((v), (hi)), (lo))

#define EFFECT_TIME 30.0

struct _opc_client
{
  int                 fd;
  struct sockaddr_in  address;
  int                 fb_size;
  double             *framebuffer;
};

typedef struct _opc_client OpcClient;


OpcClient * opc_client_new      (char   *hostport,
                                 int     default_port,
                                 int     fb_size,
                                 double *framebuffer);
int         opc_client_connect  (OpcClient *client);
int         opc_client_write    (OpcClient *client,
                                 uint8_t channel,
                                 uint8_t command);
void        opc_client_shutdown (OpcClient *client);

#endif
