 /*
   A very simple TCP socket server for v4 or v6
   From https://www.ibm.com/docs/en/zos/2.1.0?topic=applications-example-simple-tcp-server-program-enabled-ipv6
  */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int main(int argc,const char **argv)
{
  int serverPort = 5000;
  int rc;
  union {
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
  } serverSa; 
  union {
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
  } clientSa;

  int clientSaSize = sizeof(clientSa);
  int on = 1;
  int family;
  socklen_t serverSaSize;
  int c;
  char buf[INET6_ADDRSTRLEN];

  int s = socket(PF_INET6,SOCK_STREAM,0);
  if (s < 0)
  {
    fprintf(stderr, "IPv6 not active, falling back to IPv4...\n");
    s = socket(PF_INET,SOCK_STREAM,0);
    if (s < 0)
    {
      perror("socket failed");
      exit (1);
    }
    family = AF_INET;
    serverSaSize = sizeof(struct sockaddr_in);
  }
 else  /* got a v6 socket */
  {
    family = AF_INET6;
    serverSaSize = sizeof(struct sockaddr_in6);
  }
  printf("socket descriptor is TRUNCATED ON IBM's page");
  rc = setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&on,sizeof on);

  /* initialize the server's sockaddr */
  memset(&serverSa,0,sizeof(serverSa));
  switch(family)
  {
    case AF_INET:
      serverSa.sin.sin_family = AF_INET;
// INADDR_ANY is 0, so there is no reason to call htonl 
//     serverSa.sin.sin_addr.s_addr = htonl(INADDR_ANY);
      serverSa.sin.sin_addr.s_addr = INADDR_ANY;
      serverSa.sin.sin_port = htons(serverPort);
      break;
    case AF_INET6:
      serverSa.sin6.sin6_family = AF_INET6;
      serverSa.sin6.sin6_addr = in6addr_any;
      serverSa.sin6.sin6_port = htons(serverPort);
  }
  
    rc = bind(s,(struct sockaddr *)&serverSa,serverSaSize);
  if (rc < 0)
  {
    perror("bind failed");
    exit(1);
  }
  rc = listen(s,10);
  if (rc < 0)
  {
    perror("listen failed");
    exit(1);
  }
  rc = accept(s,(struct sockaddr *)&clientSa,&clientSaSize);
  if (rc < 0)
  {
    perror("accept failed");
    exit(1);
  }
  c = rc;
  printf("Client address is: %s\n",
       inet_ntop(clientSa.sin.sin_family,
                 clientSa.sin.sin_family == AF_INET
          // This is going to generate a warning because sin_addr is a uint32_t
          // and sin6_addr is a char *
                   ? &clientSa.sin.sin_addr
                   : &clientSa.sin6.sin6_addr,
                 buf, sizeof(buf)));

  if(clientSa.sin.sin_family == AF_INET6
    && ! IN6_IS_ADDR_V4MAPPED(&clientSa.sin6.sin6_addr))
    printf("Client is v6\n");
  else
    printf("Client is v4\n");

  rc = write(c,"hello\n",6);
  close (s);
  close (c);
  return 0;
}
