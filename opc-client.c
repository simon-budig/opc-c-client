#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <netdb.h>
#include <math.h>
#include <sys/time.h>

#include "opc-client.h"

OpcClient *
opc_client_new (char   *hostport,
                int     default_port,
                int     fb_size,
                double *framebuffer)
{
  char *host, *colon;
  int port;
  int success = 0;

  OpcClient *client = calloc (1, sizeof (OpcClient));

  client->fd = -1;
  client->fb_size = fb_size;
  client->framebuffer = framebuffer;

  host = strdup (hostport);
  colon = strchr (host, ':');
  port = default_port;

  if (colon)
    {
      *colon = '\0';
      port = strtol (colon + 1, 0, 10);
    }

  if (port)
    {
      struct addrinfo *addr, *i;
      getaddrinfo (*host ? host : "localhost", 0, 0, &addr);

      for (i = addr; i; i = i->ai_next)
        {
          if (i->ai_family == PF_INET)
            {
              memcpy (&client->address, i->ai_addr, sizeof (client->address));
              client->address.sin_port = htons (port);
              success = 1;
              break;
            }
        }

      freeaddrinfo (addr);
    }

  free (host);
  if (!success)
    {
      free (client);
      client = NULL;
    }

  return client;
}


int
opc_client_connect (OpcClient *client)
{
  int flag;

  client->fd = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (connect (client->fd,
               (struct sockaddr *) &client->address,
               sizeof (client->address)) < 0)
    {
      opc_client_shutdown (client);
      return 0;
    }

  flag = 1;
  setsockopt (client->fd,
              IPPROTO_TCP,
              TCP_NODELAY,
              (char *) &flag,
              sizeof (flag));

  signal (SIGPIPE, SIG_IGN);

  return 1;
}


int
opc_client_write (OpcClient *client,
                  uint8_t channel,
                  uint8_t command)
{
  int length = 4 + client->fb_size * sizeof (uint8_t);
  uint8_t *buffer;
  uint8_t *data;
  int i;
 
  if (client->fd < 0)
    return 0;

  buffer = malloc (length);

  buffer[0] = command;
  buffer[1] = channel;
  buffer[2] = (length - 4) >> 8;
  buffer[3] = (length - 4) & 0xff;

  for (i = 0; i < client->fb_size; i++)
    {
      buffer[i + 4] = (uint8_t) (client->framebuffer[i] * 255.0);
    }

  data = (uint8_t *) buffer;

  while (length > 0)
    {
      int res;

      res = send (client->fd, data, length, 0);
      if (res < 0)
        {
          opc_client_shutdown (client);
          return 0;
        }

      length -= res;
      data += length;
    }

  free (buffer);
  return 1;
}


void
opc_client_shutdown (OpcClient *client)
{
  if (client->fd >= 0)
    {
      close (client->fd);
      client->fd = -1;
    }
}


