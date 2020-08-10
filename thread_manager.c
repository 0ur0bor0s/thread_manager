#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "command_node.h"

//thread mutex lock for access to the log index
pthread_mutex_t tlock1 = PTHREAD_MUTEX_INITIALIZER;
//thread mutex lock for critical sections of allocating THREADDATA
pthread_mutex_t tlock2 = PTHREAD_MUTEX_INITIALIZER;
// thread mutex lock for the access of the thread
pthread_mutex_t tlock3 = PTHREAD_MUTEX_INITIALIZER;


pthread_mutex_t tlock4 = PTHREAD_MUTEX_INITIALIZER;

// condition variable for link node update
pthread_cond_t ll_update_cond = PTHREAD_COND_INITIALIZER;


void *thread_runner(void *);
void print_log(pthread_t me, char* msg, char* arg);

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

// variables needed for linked list
int arr_size = 20;
char** cmd_str_arr;
CommandNode** cmd_node_arr;
bool is_first_line = true;

// pointer to linked list head
CommandNode** head_ref;


/*********************************************************
// function main  ------------------------------------------------- 
*********************************************************/
int main(int argc, char **argv) {

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

    // print log
    print_log(me, "THREAD RUNNER ENTERED", NULL);

    // create THREADATA object
    pthread_mutex_lock(&tlock2); // critical section starts
    if (p == NULL) {
        p = (THREADDATA *) malloc(sizeof(THREADDATA));
        p->creator = me;
        // print log
        print_log(me, "INITIALIZED THREADDATA", NULL);
    }
    pthread_mutex_unlock(&tlock2);  // critical section ends

    if (p != NULL && p->creator == me) {

        signal(SIGINT, signal_handler);

        char buf[20];
        char* line;
        int node_index = 0;
        while (!is_reading_complete) {
            line = fgets(buf, 21, stdin);

            // exit for end of file as well as SIGINT
            // this is kind of hacky but is_reading_complete is checked to tell if SIGINT was sent
            if (line == NULL || is_reading_complete) {

                // print log
                if (line == NULL)
                    print_log(me, "END OF FILE SIGNAL RECEIVED", NULL);
                else
                    print_log(me, "SIGINT SIGNAL RECEIVED", NULL);

                // set complete flag to true
                is_reading_complete = true;

                // send signal to thread 2
                pthread_cond_signal(&ll_update_cond);

                // deallocate linked list memory
                //pthread_mutex_lock(&tlock3);
                //free(cmd_node_arr);
                //free(cmd_str_arr);
                //pthread_mutex_unlock(&tlock3);

                // print log
                //print_log(me, "DEALLOCATED LINKED LIST", NULL);

            } else { // proceed further
                pthread_mutex_lock(&tlock3);

                // if first line first allocate memory for data
                if (is_first_line) {
                    // print log
                    print_log(me, "INITIALIZED LINKED LIST", NULL);
                    cmd_str_arr = (char**) malloc(arr_size * sizeof(char*));
                    cmd_node_arr = (CommandNode**) malloc(arr_size * sizeof(CommandNode*));
                    is_first_line = false;
                } else if (node_index + 1 > arr_size) {  // allocate more data when needed
                    // print log
                    print_log(me, "EXPANDED LINKED LIST MEMORY", NULL);
                    cmd_str_arr = realloc(cmd_str_arr, ++arr_size * sizeof(char*));
                    cmd_node_arr = realloc(cmd_node_arr, arr_size * sizeof(CommandNode*));
                }

                // allocate memory to the char row
                cmd_str_arr[node_index] = (char*) malloc(strlen(buf) + 1);

                // copy contents to the char array
                buf[strlen(buf)-1] = '\0';
                strcpy(cmd_str_arr[node_index], buf);

                // create linked list node
                cmd_node_arr[node_index] = (CommandNode*) malloc(sizeof(CommandNode));
                CreateCommandNode(cmd_node_arr[node_index], cmd_str_arr[node_index], node_index, NULL);

                // push new node to head
                if (node_index != 0) {
                    PushCommand(cmd_node_arr[node_index], head_ref);
                } else { // point head_ref to first node head
                    head_ref = &cmd_node_arr[0];
                }

                // print log
                print_log(me, "CREATED AND INSERTED NEW NODE", NULL);

                pthread_cond_signal(&ll_update_cond);
                pthread_mutex_unlock(&tlock3);
                // increment index
                ++node_index;
            }
        }

    } else {
            // print contents of first node when it is updated
            while (1) {
                // lock critical printing linked list section
                pthread_mutex_lock(&tlock3);
                // wait for linked list to update
                pthread_cond_wait(&ll_update_cond, &tlock3);
                if (is_reading_complete) {
                    break;
                } else {
                    print_log(me, "HEAD OF LINKED LIST CONTAINS LINE:", (*head_ref)->data);
                }
                pthread_mutex_unlock(&tlock3);
            }
    }

    // TODO use mutex to make this a start of a critical section
    pthread_mutex_lock(&tlock2);
    if (p != NULL && p->creator == me) {
        // deallocate linked list memory
        pthread_mutex_lock(&tlock3);
        free(cmd_node_arr);
        free(cmd_str_arr);
        pthread_mutex_unlock(&tlock3);

        // print log
        print_log(me, "DEALLOCATED LINKED LIST", NULL);
    } else {
        /**
         * TODO Free the THREADATA object. Freeing should be done by the other thread from the one that created it.
         * See how the THREADDATA was created for an example of how this is done.
         */
        free(p);
    }
    // TODO critical section ends
    pthread_mutex_unlock(&tlock2);

    // print log
    print_log(me, "EXITING THREAD RUNNER", NULL);

    pthread_exit(NULL);
    return NULL;

}//end thread_runner


/**********************************************************************
// function prints log --------------------------------------------------
**********************************************************************/
void print_log(pthread_t me, char* msg, char* arg) {
    // get time
    time_t now;
    time(&now);

    struct tm *local = localtime(&now);

    int hours = local->tm_hour;
    int minutes = local->tm_min;
    int seconds = local->tm_sec;

    int day = local->tm_mday;
    int month = local->tm_mon + 1;
    int year = local->tm_year + 1900;

    pthread_mutex_lock(&tlock1);
    if (hours < 12) { // AM
        if (arg == NULL)
            printf("Logindex %d, thread %ld (p=%p), PID %d, %02d/%02d/%02d %02d:%02d:%02d am: %s.\n",
                    logindex++, me, p, getpid(), month, day, year, hours, minutes, seconds, msg);
        else // print with arg
            printf("Logindex %d, thread %ld (p=%p), PID %d, %02d/%02d/%02d %02d:%02d:%02d am: %s %s.\n",
                   logindex++, me, p, getpid(), month, day, year, hours, minutes, seconds, msg, arg);
    } else { // PM

        // convert from 24 hour clock to 12 hour clock
        if (hours > 12)
            hours -= 12;

        if (arg == NULL)
            printf("Logindex %d, thread %ld (p=%p), PID %d, %02d/%02d/%02d %02d:%02d:%02d pm: %s.\n",
                    logindex++, me, p, getpid(), month, day, year, hours, minutes, seconds, msg);
        else // print with arg
            printf("Logindex %d, thread %ld (p=%p), PID %d, %02d/%02d/%02d %02d:%02d:%02d pm: %s %s.\n",
                   logindex++, me, p, getpid(), month, day, year, hours, minutes, seconds, msg, arg);
    }
    pthread_mutex_unlock(&tlock1);
}
