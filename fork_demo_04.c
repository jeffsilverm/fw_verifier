#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_CHILDREN    20

int annotate(int status, char *generation){
    if ( status == 0 ){
        printf("In a child: my PID is %d  and PPID is %d\n", getpid(), getppid() );
        sleep(30.0);
        printf("Child process %d is exiting\n", getpid() ); }
    else if ( status > 0 ) {
        printf("Just forked %s child PID %d, my PID is %d  and PPID is %d\n", generation, status, getpid(), getppid() );
    }
    else {
        printf("Something awful happened!  status is %d\n", status );
        exit(status);
    }
}
    

int main()
{
    int status;
    char annotation[1000];
    for ( int i=1; i++; i< MAX_CHILDREN ) {
      sprintf ( annotation, "%u\n", i);
      status = fork();
      annotate( status, annotation );
      printf("I have now forked %d children\n", i);
    };
    return 0;
}
