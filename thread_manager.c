#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>

#include "command_node.h"

//thread mutex lock for access to the log index
pthread_mutex_t tlock1 = PTHREAD_MUTEX_INITIALIZER;
//thread mutex lock for critical sections of allocating THREADDATA
pthread_mutex_t tlock2 = PTHREAD_MUTEX_INITIALIZER;
// thread mutex lock for the access of the thread
pthread_mutex_t tlock3 = PTHREAD_MUTEX_INITIALIZER;


void *thread_runner(void *);

pthread_t tid1, tid2;
struct THREADDATA_STRUCT {
    pthread_t creator;
};
typedef struct THREADDATA_STRUCT THREADDATA;

THREADDATA *p = NULL;


//variable for indexing of messages by the logging function
int logindex = 0;
int *logip = &logindex;


//A flag to indicate if the reading of input is complete, 
//so the other thread knows when to stop
bool is_reading_complete = false;

// variables needed for linke list
int arr_size = 20;
char** cmd_str_arr;
CommandNode** cmd_node_arr;
bool is_first_line = true;


/*********************************************************
// function main  ------------------------------------------------- 
*********************************************************/
int main(int argc, char **argv) {

    //int arr_size = 20;
    //MyArgs args;
    //args.cmd_str_arr = (char**) malloc(arr_size * sizeof(char*));
    //args.cmd_node_arr = (CommandNode**) malloc(arr_size * sizeof(CommandNode*));

    printf("create first thread\n");
    pthread_create(&tid1, NULL, thread_runner, NULL);

    printf("create second thread\n");
    pthread_create(&tid2, NULL, thread_runner, NULL);

    printf("wait for first thread to exit\n");
    pthread_join(tid1, NULL);
    printf("first thread exited\n");

    printf("wait for second thread to exit\n");
    pthread_join(tid2, NULL);
    printf("second thread exited\n");

    exit(0);
}//end main

// Handler for signals
void signal_handler() {
    is_reading_complete = true;
}

/**********************************************************************
// function thread_runner runs inside each thread --------------------------------------------------
**********************************************************************/
void *thread_runner(void *x) {
    pthread_t me;

    me = pthread_self();
    printf("This is thread %ld (p=%p)\n", me, p);

    // create THREADATA object
    pthread_mutex_lock(&tlock2); // critical section starts
    if (p == NULL) {
        p = (THREADDATA *) malloc(sizeof(THREADDATA));
        p->creator = me;
    }
    pthread_mutex_unlock(&tlock2);  // critical section ends

    if (p != NULL && p->creator == me) {
        printf("This is thread %ld and I created the THREADDATA %p\n", me, p);

        // ctrl-c = SIGINT
        char buf[20];
        char* line;
        while (!is_reading_complete) {
            line = fgets(buf, 21, stdin);

            // exit for termination or end of file signal
            if (signal(SIGINT, signal_handler) == SIG_ERR || line == NULL) {
                is_reading_complete = true;
            }

            pthread_mutex_lock(&tlock3);
            // if first line first allocate memory for data
            if (is_first_line) {
                cmd_str_arr = (char**) malloc(arr_size * sizeof(char*));
                cmd_node_arr = (CommandNode**) malloc(arr_size * sizeof(CommandNode*));
                is_first_line = false;
            } else if (logindex + 1 > arr_size) {  // alocate more data when needed
                cmd_str_arr = realloc(cmd_str_arr, ++arr_size * sizeof(char*));
                cmd_node_arr = realloc(cmd_node_arr, arr_size * sizeof(CommandNode*));
            }

            // allocate memory to the char row
            cmd_str_arr[logindex] = (char*) malloc(strlen(buf) + 1);

            // copy contents to the char array
            strcpy(cmd_str_arr[logindex], buf);

            // create linked list node
            cmd_node_arr[logindex] = (CommandNode*) malloc(sizeof(CommandNode));
            CreateCommandNode(cmd_node_arr[logindex], cmd_str_arr[logindex], logindex, NULL);

            // insert trailing nodes after previous one
            if (logindex != 0) {
                InsertCommandAfter(cmd_node_arr[logindex-1], cmd_node_arr[logindex]);
            }

            pthread_mutex_unlock(&tlock3);

            pthread_mutex_lock(&tlock1);
            logindex++;
            pthread_mutex_unlock(&tlock1);
        }


    } else {
        printf("This is thread %ld and I can access the THREADDATA %p\n", me, p);

    }

    // TODO use mutex to make this a start of a critical section
    pthread_mutex_lock(&tlock2);
    if (p != NULL && p->creator == me)
        printf("This is thread %ld and I did not touch THREADDATA\n", me);
    else {
        /**
         * TODO Free the THREADATA object. Freeing should be done by the other thread from the one that created it.
         * See how the THREADDATA was created for an example of how this is done.
         */
        //free(p);
        printf("This is thread %ld and I deleted the THREADDATA\n", me);
    }
    // TODO critical section ends
    pthread_mutex_unlock(&tlock2);

    pthread_exit(NULL);
    return NULL;

}//end thread_runner



