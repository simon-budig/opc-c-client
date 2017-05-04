#ifndef __OPC_CLIENT_H__
#define __OPC_CLIENT_H__

struct _opc_client
{
  int                 fd;
  struct addrinfo    *addresses;
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
