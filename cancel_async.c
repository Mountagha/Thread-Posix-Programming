#include <pthread.h>
#include "errors.h"

#define SIZE    10 // array size

static int matrixa[SIZE][SIZE];
static int matrixb[SIZE][SIZE];
static int matrixc[SIZE][SIZE];

/* 
 * Loop until canceled. the thread can be canceled at any 
 * point within the ineer loop, where asynchronous cancellation
 * is enabled. The loop multiplies the two matrices matrixa and 
 * matrixb.
 */

void *thread_routine(void *arg){
    int cancel_type, status;
    int i, j, k, value = 1;

    /* Initialize the matrices to something arbitrary. */
    for (i = 0; i<SIZE; i++)
        for(j = 0; j<SIZE; j++){
            matrixa[i][j] = i;
            matrixb[i][j] = j;
        }
    while(1){
        // compute the matrix product of matrixa and matrixb
        status = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &cancel_type);
        if (status != 0)
            err_abort(status, "Set cancel type");
        for (i = 0; i<SIZE; i++)
            for(j = 0; j<SIZE; j++){
                matrixc[i][j] = 0;
                for(k = 0; k<SIZE; k++)
                    matrixc[i][j] += matrixa[i][k] * matrixb[k][j];
            }
        status = pthread_setcanceltype(cancel_type, &cancel_type);
        if (status != 0)
            err_abort(status, "Set cancel type");
        // Copy the result (matrixc) into matrixa to start again
        for(i = 0; i<SIZE; i++)
            for(j = 0; j<SIZE; j++)
                matrixa[i][j] = matrixc[i][j];
    }
}

int main(int argc, char *argv[]){
    pthread_t thread_id;
    void *result;
    int status;

    status = pthread_create(&thread_id, NULL, thread_routine, NULL);
    if (status != 0)
        err_abort(status, "Create thread");
    sleep(1);

    status = pthread_cancel(thread_id);
    if (status != 0)
        err_abort(status, "cancel thread");
    status = pthread_join(thread_id, &result);
    if (status != 0)
        err_abort(status, "join thread");
    if (result == PTHREAD_CANCELED)
        printf("Thread canceled\n");
    else 
        printf("thread was not canceled\n");
    return 0;
}