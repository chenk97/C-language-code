#include <stdlib.h>
#include <server.h>
#include "client_registry.h"
#include "exchange.h"
#include "trader.h"
#include "debug.h"
#include "csapp.h"

#include <unistd.h>
#include <signal.h>

extern EXCHANGE *exchange;
extern CLIENT_REGISTRY *client_registry;

static void terminate(int status);

void termination_handler (int signum);

/*
 * "Bourse" exchange server.
 *
 * Usage: bourse <port>
 */
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

    int listenfd, *connfdp, port;
    socklen_t clientlen = sizeof(struct sockaddr_storage);
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    if (argc != 3){
        fprintf(stderr, "usage : %s -p <port>\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[2]);

    // Perform required initializations of the client_registry,
    // maze, and player modules.
    client_registry = creg_init();
    exchange = exchange_init();
    trader_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function brs_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    struct sigaction new_action, old_action;

    /* Set up the structure to specify the new action. */
    new_action.sa_handler = termination_handler;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction (SIGHUP, NULL, &old_action);/*retrieve original action*/
    if (old_action.sa_handler != SIG_IGN) sigaction (SIGHUP, &new_action, NULL);/*install new action*/

    listenfd = Open_listenfd(port);
    while(1){
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        Pthread_create(&tid, NULL, brs_client_service, connfdp);
    }
    // terminate(EXIT_FAILURE);
}


void termination_handler (int signum){
    terminate(EXIT_SUCCESS);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    exchange_fini(exchange);
    trader_fini();

    debug("Bourse server terminating");
    exit(status);
}
