#include <pthread.h>
#include "errors.h"

static int counter;

/* 
 * Loop until canceled. The thread can be canceled only
 * when it call pthread_testcancel, which it does each 1000 iterations.
 */

void *thread_rountine(void *arg){
    DPRINTF(("Thread_routine starting\n"));
    for(counter = 0; ; counter++)
        if ((counter % 1000) == 0){
            DPRINTF(("calling testcancel\n"))
            pthread_testcancel();
        }
}

int main(int argc, char *argv[]){
    pthread_t thread_id;
    void *result;
    int status;

    status = pthread_create(&thread_id, NULL, thread_rountine, NULL);
    if (status != 0)
        err_abort(status, "Create thread");
    sleep(2);

    DPRINTF(("calling cancel\n"));
    status = pthread_cancel(thread_id);
    if (status != 0)
        err_abort(status, "Cancel thread");
    DPRINTF(("Calling join\n"));
    status = pthread_join(thread_id, &result);
    if (status != 0)
        err_abort(status, "Join thread");
    if (result == PTHREAD_CANCELED)
        printf("Thread canceled at iteration %d\n", counter);
    else
        printf("Thread was not canceled");
    return 0;
}