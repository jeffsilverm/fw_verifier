#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>

#define MAX_CHILDREN    50000

int annotate(int status, char *generation){
    if ( status == 0 ){
        printf("In a child: my PID is %d  and PPID is %d.  Genertion is %s\n",
            getpid(), getppid(), generation );
        sleep(60.0);
        printf("Child process %d is exiting and waiting for the parent to reap.\n", getpid() );
        fflush(stdout);
        exit(0);
        }
    else if ( status > 0 ) {
        printf("Just forked %s child PID %d, my PID is %d  and PPID is %d\n", generation, status, getpid(), getppid() );
        fflush(stdout);
    }
    else {
        printf("Something awful happened!  status is %d\n", status );
        fflush(stdout);
        exit(status);
    };
}
    

int main()
{
    int status;
    char annotation[1000];
    int i=1;
    while ( i< MAX_CHILDREN ) {
      i++;
      sprintf ( annotation, "%u\n", i);
      status = fork();
      annotate( status, annotation );
      printf("I have now forked %d children\n", i);
      if ( i > MAX_CHILDREN * 2 ) {
          printf("I have forked too many children %d now I am going to break\n", i);
          fflush(stdout);
          break;
      } else {
          fflush(stdout);
      }
    };
    printf("Now reaping children\n");
    while ( 1 ) {
        pid_t wstatus;
        printf("Waiting for any child to finish\n");
        wstatus = waitpid(-1, 0, 0);
        if ( wstatus < 0 && errno == ECHILD ) {
            printf("Done reaping children\n");
            break;
        } else {
            printf("Child %d has finished\n", wstatus);
        };

    }
    return 0;
}
