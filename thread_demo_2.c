#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

// The do..while doesn't actually do anything, they are just there to make sure that the
// macro expanstions aren't broken by the code around them.
#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define MAX_NUM_THREADS 6

struct thread_info {    /* Used as argument to thread_start() */
    pthread_t thread_id;        /* ID returned by pthread_create() */
    int       thread_num;       /* Application-defined thread # */
    char     *argv_string;     
};

/* Thread start function: display a number and then wait to join */

static void *
thread_start_routine(void *arg)
{
    struct thread_info *tinfo = arg;
    

    printf("Thread %d:\n", tinfo->thread_num);

    // cast a pointer to the result of an integer expression
    tinfo->thread_num += 100000;
   return (void *) &( tinfo->thread_num );
}

int main(int argc, char *argv[]) {

    pthread_t tid_table[MAX_NUM_THREADS];
    int i;
    int status;
    struct thread_info arg_table[MAX_NUM_THREADS];
    void *res;      // the results from the thread will be returned here.

    for (i=0; i<MAX_NUM_THREADS; i++ ){
        /* Create a thread.  tid_table is a table of tids, indexed by i.  The argument
        is actually a pointer to a tid, hence adding i (the compiler will automatically
        scale i to whatever the size of a pthread_t is).  pthread_create will call 
        thread_start_routine in the context of the new thread.  In this case, to keep
        things simple, I am not using any thread attributes
        */
        arg_table[i].thread_num = i;   // The thread is going to get its own number
        status = pthread_create( tid_table + i, NULL, &thread_start_routine, arg_table + i  );
        if ( status != 0)
            handle_error_en(status, "pthread_create");
        printf("Created thread %d\n", i);
    }
    printf("All %d threads created.\nWaiting...", MAX_NUM_THREADS);
    sleep(15.0);
    printf("\nNow start joining them.\n");
    

    /* Now join with each thread, and display its returned value */
    for (i=0; i < MAX_NUM_THREADS; i++) {
        status = pthread_join( tid_table[i], &res);
        if (status != 0)
            handle_error_en(status, "pthread_join");

        printf("Joined with thread %d; returned value was %d\n",
                i, *(int *)res );
    }


    exit(EXIT_SUCCESS);
}
