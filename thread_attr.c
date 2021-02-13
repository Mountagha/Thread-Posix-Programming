#include <limits.h>
#include <pthread.h>
#include "errors.h"

// Thread start routine that reports it ran and then exits

void *routine(void *arg){
    printf("The thread is here\n");
    return NULL;
}

int main(int argc, char *argv[]){
    pthread_t thread_id;
    pthread_attr_t thread_attr;
    struct sched_param thread_param;
    size_t stack_size;
    int status;

    status = pthread_attr_init(&thread_attr);
    if (status != 0)
        err_abort(status, "Create attr");
    
    // Create a detached thread.
    status = pthread_attr_setdetachstate(
        &thread_attr, PTHREAD_CREATE_DETACHED
    );
    if (status != 0)
        err_abort(status, "Set detach");
#ifdef _POSIX_THREAD_ATTR_STACKSIZE
    /*
     * If supported, determined the default stack size and report
     * it, and then select a stack size for the new thread.
     * 
     * Note that the standard does not specify the default stack 
     * size, and the default value in an attributes object need
     * not be the size that will actually be used. Solaris 2.5 
     * uses a value of 0 to indicate the default.
     */
    status = pthread_attr_getstacksize(&thread_attr, &stack_size);
    if (status != 0)
        err_abort(status, "Get stack size");
    printf("Default stack size is %lu; minimum is %u\n",
    stack_size, PTHREAD_STACK_MIN);
    status = pthread_attr_setstacksize(&thread_attr, PTHREAD_STACK_MIN*2);
    if (status != 0)
        err_abort(status, "Set stack size");
#endif
    status = pthread_create(
        &thread_id, &thread_attr, routine, NULL);
    if (status != 0)
        err_abort(status, "Create thread");
    printf("Main exiting\n");
    pthread_exit(NULL);
    return 0;
}