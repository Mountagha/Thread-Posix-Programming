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

// Destroy a barrier when done using it.

int barrier_destroy (barrier_t *barrier){
    int status, status2;

    if (barrier->valid != BARRIER_VALID){
        return EINVAL;
    }
    status = pthread_mutex_lock(&barrier->mutex);
    if (status != 0)
        return status;
    // check whether any threads are known to be waiting; report "BUSY" if so.
    if (barrier->counter != barrier->threshold){
        pthread_mutex_unlock(&barrier->mutex);
        return EBUSY;
    }

    barrier->valid = 0;
    status = pthread_mutex_unlock (&barrier->mutex);
    if (status != 0)
        return status;
    
    // If unable to destroy either 1003.lc synchronization object, return the error status
    status = pthread_mutex_destroy (&barrier->mutex);
    status2 = pthread_cond_destroy (&barrier->cv);
    return (status == 0 ? status : status2);
}

/*
 * Wait for all memberes of a barrier to reach the barrier. When 
 * the count (of remining members) reaches 0, broadcast to wake 
 * all threads waiting.
 */

int barrier_wait (barrier_t *barrier) {
    int status, cancel, tmp, cycle;

    if (barrier->valid != BARRIER_VALID)
        return EINVAL;
    status = pthread_mutex_lock (&barrier->mutex);
    if (status != 0)
        return status;
    cycle = barrier->cycle; // Remember which cycle we're on 

    if (--barrier->counter == 0) {
        barrier->cycle = !barrier->cycle;
        barrier->counter = barrier->threshold;
        status = pthread_cond_broadcast (&barrier->cv);
        /*
         * The last thread into the barrier will return status
         * -1 rather than 0, so that it can be used to perform
         * some special serial code following the barrier.
         */
        if (status == 0)
            status = -1;
    } else {
        /* 
         * Wait with cancellation disabled, because barrier_wait
         *  should not be a cancellation point.
         */
        pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &cancel);

        /*
         * Wait until the barrier's cycle changes, which means
         * that it has been broadcast, and we don't want to wait
         * anymore.
         */
        
        while (cycle == barrier->cycle) {
            status = pthread_cond_wait (
                &barrier->cv, &barrier->mutex
            );
            if (status != 0) break;
        }

        pthread_setcancelstate(cancel, &tmp);
    }

    /*
     * Ignore an error in unlocking. It shouldn't happen, and
     * reporting it here would be misleading -- the barrier wait
     * completed, after all, whereas returning, for example, 
     * EINVAL would imply the wait had failed. The next attempt
     * to use the barrier *will* return an error, or hang, due 
     * to whatever happened to the mutex.
     */

    pthread_mutex_unlock (&barrier->mutex);
    return status;              // error, -1 for waker or 0
}

