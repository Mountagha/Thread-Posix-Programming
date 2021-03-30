#include <pthread.h>
#include "errors.h"
#include "barrier.h"

// Initialize a barrier for use

int barrier_init (barrier_t *barrier, int count){

    int status;

    barrier->threshold = barrier->counter = count;
    barrier->cycle = 0;
    status = pthread_mutex_init (&barrier->mutex, NULL);
    if (status != 0)
        return status;
    status = pthread_cond_init (&barrier->cv, NULL);
    if (status != 0){
        pthread_mutex_destroy(&barrier->mutex);
        return status;
    }
    barrier->valid = BARRIER_VALID;
    return 0;
}