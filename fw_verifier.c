#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

#define MAX_CHILDREN 50000

// The do..while doesn't actually do anything, they are just there to make sure that the
// macro expanstions aren't broken by the code around them.
#define handle_error_en(en, msg) \
  do                             \
  {                              \
    errno = en;                  \
    perror(msg);                 \
    exit(EXIT_FAILURE);          \
  } while (0)

#define handle_error(msg) \
  do                      \
  {                       \
    perror(msg);          \
    exit(EXIT_FAILURE);   \
  } while (0)

#define MAX_NUM_THREADS 30000

struct thread_info
{                      /* Used as argument to thread_start() */
  pthread_t thread_id; /* ID returned by pthread_create() */
  int thread_num;      /* Application-defined thread # */
  char *argv_string;
};

int opensocket(int sockPort);

/* Thread start function: open a TCP port and then wait to joined from the parent */
static void *thread_start_routine(void *arg)
{
  struct thread_info *tinfo = arg; // This is subtle.  tinfo now points to the same struct arg points to
  int port = tinfo->thread_num;    // tinfo points to a thread_num field in the struct that arg would point to
                                   // if arg was a struct thread_info, but it's a void pointer
  int status = opensocket(port);   // tinfo, port, and status are all local variables, but arg is a pointer
                                   // to a non-local variable
  tinfo->thread_num = status;      // status is going back to the caller.  Field thread_num is in a space that
                                   // pthread manages.
  return (void *)arg;
}

int main(int argc, char **argv)
{
  int status;
  char annotation[1000];
  int lower_limit = atoi(argv[1]);
  int upper_limit = atoi(argv[2]);
  int port;
  int ctr;
  pthread_t tid_table[MAX_NUM_THREADS];
  struct thread_info arg_table[MAX_NUM_THREADS]; // thread_info is application defined
  void *res;                                     // the results from the thread will be returned here.

  if ((upper_limit - lower_limit) > MAX_NUM_THREADS)
  {
    fprintf(stderr, "Upper limit is %d, lower limit is %d the difference is %d which is more than %d\n",
            upper_limit, lower_limit, upper_limit - lower_limit, MAX_NUM_THREADS);
    exit(-20);
  }

  ctr = 0;
  // Create a bunch of threads.  Each thread will open one port and hold it open until reaped/
  for (port = lower_limit; port <= upper_limit; port++)
  {
    // The thread has to know which port to use.
    arg_table[ctr].thread_num = port;
    status = pthread_create(tid_table + ctr, NULL, &thread_start_routine, arg_table + ctr);
    if (status != 0)
      handle_error_en(status, "pthread_create");
    printf("Created thread %d\n", ctr);
    ctr++;
  }

  /*
  status = opensocket(port);
  if (status != 0)
  {
    printf("Port %d FAILED\n", port);
  }
  else
  {
    putc('.', stdout);
  };
  putc('\n', stdout); */
  printf("Waiting for you to press enter\n");
  fflush(stdout);

  status = getchar();
  printf("Now reaping children\n");
  while (1)
  {
    pid_t wstatus;
    printf("Waiting for any child to finish\n");
    wstatus = waitpid(-1, 0, 0);
    if (wstatus < 0 && errno == ECHILD)
    {
      printf("Done reaping children\n");
      break;
    }
    else
    {
      printf("Child %d has finished\n", wstatus);
    };
  }
  return 0;
}

int opensocket(const int serverPort)
{
  int rc;
  union
  {
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
  } serverSa;
  union
  {
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
  } clientSa;

  int clientSaSize = sizeof(clientSa);
  int on = 1;
  int family;
  socklen_t serverSaSize;
  int c;
  char buf[INET6_ADDRSTRLEN];
  char wb[10000]; // a working buffer for, well, just about any short term string manipulation

  int s = socket(PF_INET6, SOCK_STREAM, 0); // s a file descriptor, suitable for read, write, listen or accept syscalls
  if (s < 0)
  {
    fprintf(stderr, "IPv6 not active, falling back to IPv4...\n");
    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
      perror("socket failed");
      return -1;
    }
    family = AF_INET;
    serverSaSize = sizeof(struct sockaddr_in);
  }
  else /* got a v6 socket */
  {
    family = AF_INET6;
    serverSaSize = sizeof(struct sockaddr_in6);
  }
  /*

int setsockopt(int         socket,
               int         level,
               int         option_name,
               const void *option value,
               size_t      option_length);

struct linger is defined in /usr/include/bits/socket.h on my Ubuntu 20.04 machine.
The description is:
When enabled, a close(2) or shutdown(2) will not return until all queued messages for
the socket have been successfully sent or the linger timeout has been reached.
Otherwise, the call returns immediately and the closing is done in the background.
When the socket is closed as part of exit(2), it always lingers in the background.

In https://stackoverflow.com/questions/3757289/when-is-tcp-option-so-linger-0-required
there is a reference in https://stackoverflow.com/questions/3757289/when-is-tcp-option-so-linger-0-required

If you must restart your server application which currently has thousands of client connections
you might consider setting this socket option to avoid thousands of server sockets in TIME_WAIT
(when calling close() from the server end) as this might prevent the server from getting available
ports for new client connections after being restarted.
*/
  struct linger linger;
  linger.l_onoff = 0;
  linger.l_linger = 0;

  rc = setsockopt(s, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));

  // Typo on IBM's page, https://www.ibm.com/docs/en/zos/2.1.0?topic=applications-example-simple-tcp-server-program-enabled-ipv6
  // fprintf(stderr, "socket descriptor is TRUNCATED ON IBM's page\n");
  //
  /* SO_REUSEPORT (since Linux 3.9)
          Permits multiple AF_INET or AF_INET6 sockets to be bound to an
          identical socket address.  This option must be set on each
          socket (including the first socket) prior to calling bind(2)
          on the socket.  To prevent port hijacking, all of the
          processes binding to the same address must have the same
          effective UID.  This option can be employed with both TCP and
          UDP sockets. 
*/
  rc = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);

  /* initialize the server's sockaddr */
  memset(&serverSa, 0, sizeof(serverSa));
  switch (family)
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

  fprintf(stderr, "Binding to port %d\n", serverPort);
  rc = bind(s, (struct sockaddr *)&serverSa, serverSaSize);
  if (rc < 0)
  {
    sprintf(wb, "bind failed on port %d", serverPort);
    perror(wb);
    return -2;
  }
  rc = listen(s, 10);
  if (rc < 0)
  {
    perror("listen failed");
    return -3;
  }
  rc = accept(s, (struct sockaddr *)&clientSa, &clientSaSize);
  if (rc < 0)
  {
    perror("accept failed");
    return -4;
  }
  c = rc; // c is a file descriptor, suitable for read, write, close, or shutdown
  printf("Client address is: %s\n",
         inet_ntop(clientSa.sin.sin_family,
                   clientSa.sin.sin_family == AF_INET
                       // This is going to generate a warning because sin_addr is a uint32_t
                       // and sin6_addr is a char *
                       ? &clientSa.sin.sin_addr
                       : &clientSa.sin6.sin6_addr,
                   buf, sizeof(buf)));

  if (clientSa.sin.sin_family == AF_INET6 && !IN6_IS_ADDR_V4MAPPED(&clientSa.sin6.sin6_addr))
    printf("Client is v6\n");
  else
    printf("Client is v4\n");

  rc = write(c, "hello\n", 6); // 6 is the length of "hello\n"
  shutdown(s, 2);              // 2 meams both send and receive, this was close(s) in the original code from IBM
  shutdown(c, 2);              // 2 meams both send and receive, this was close(c) in the original code from IBM
  return 0;                    // Successful exit
}
