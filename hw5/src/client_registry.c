#include "client_registry.h"
#include "csapp.h"
#include "debug.h"

typedef struct client_registry{
    int max;
    int cnt;
    int *cfd_buf;
    pthread_mutex_t mutex;
    sem_t semaphore;
}CLIENT_REGISTRY;



/*
 * Initialize a new client registry.
 *
 * @return  the newly initialized client registry, or NULL if initialization
 * fails.
 */
CLIENT_REGISTRY *creg_init(){
    CLIENT_REGISTRY *cr = (CLIENT_REGISTRY *)Malloc(sizeof(CLIENT_REGISTRY));
    cr -> max = 64;
    cr -> cnt = 0;
    cr -> cfd_buf = (int *)Malloc(cr -> max * sizeof(int));
    pthread_mutex_init(&cr -> mutex, NULL);
    Sem_init(&cr -> semaphore, 0, 1);
    return cr;
}

/*
 * Finalize a client registry, freeing all associated resources.
 *
 * @param cr  The client registry to be finalized, which must not
 * be referenced again.
 */
void creg_fini(CLIENT_REGISTRY *cr){
    free(cr -> cfd_buf);
    free(cr);
    pthread_mutex_destroy(&cr -> mutex);
}

/*
 * Register a client file descriptor.
 *
 * @param cr  The client registry.
 * @param fd  The file descriptor to be registered.
 * @return 0 if registration succeeds, otherwise -1.
 */
int creg_register(CLIENT_REGISTRY *cr, int fd){
    if(cr -> cnt >= cr -> max){
        return -1;
    }
    pthread_mutex_lock(&cr -> mutex);
    *(cr -> cfd_buf + cr -> cnt) = fd;
    debug("buf: %d", *(cr -> cfd_buf + cr -> cnt));
    cr -> cnt ++;
    debug("Register client fd %d (total connected: %d)", fd, cr -> cnt);
    pthread_mutex_unlock(&cr -> mutex);
    return 0;
}


int creg_unregister(CLIENT_REGISTRY *cr, int fd){
    debug("***********unregistered***********");
    pthread_mutex_lock(&cr -> mutex);
    //int temp = -1;
    for(int i = 0; i < cr -> cnt; i++){
        if(*(cr -> cfd_buf + i) == fd){
            for(int j = i; j < cr -> cnt; j ++){
                debug("buf: %d", *(cr -> cfd_buf + j));
                *(cr -> cfd_buf + j)=*(cr -> cfd_buf + j + 1);
            }
            cr -> cnt --;
            debug("Unregister client fd %d (total connected: %d)", fd, cr -> cnt);
            pthread_mutex_unlock(&cr -> mutex);
            return 0;
        }
    }
    pthread_mutex_unlock(&cr -> mutex);
    return -1;
}

/*
 * A thread calling this function will block in the call until
 * the number of registered clients has reached zero, at which
 * point the function will return.
 *
 * @param cr  The client registry.
 */
void creg_wait_for_empty(CLIENT_REGISTRY *cr){
    debug("**************wait for empty**************");
    P(&cr -> semaphore);
    while(1){
        if(!cr -> cnt)break;
    }
    V(&cr -> semaphore);
}


/*
 * Shut down all the currently registered client file descriptors.
 *
 * @param cr  The client registry.
 */
void creg_shutdown_all(CLIENT_REGISTRY *cr){
    pthread_mutex_lock(&cr -> mutex);
    while(cr -> cnt){
        debug("shutdown all - count: %d", cr -> cnt);
        if(shutdown(*(cr -> cfd_buf + cr -> cnt - 1), SHUT_RD) == -1){
            debug("shutdown all - fail shutdown fd[%d]", *(cr -> cfd_buf + cr -> cnt - 1));
        }
        debug("shutdown all - successfully shutdown fd[%d]", *(cr -> cfd_buf + cr -> cnt - 1));
        cr -> cnt --;
    }
    pthread_mutex_unlock(&cr->mutex);
}