#include <stdlib.h>
#include <errno.h>
#include <string.h>
// #include "jobber.h"
#include "helper.h"


/*
 * "Jobber" job spooler.
 */

int main(int argc, char *argv[])
{
    // TO BE IMPLEMENTED
    PROC_ENABLED = DISABLED;
    CURRENT_RUNNER = 0;
    sigset_t mask_all, prev_all;
    sigfillset(&mask_all);
    // sigset_t mask;
    signal(SIGCHLD, child_handler);
    jobs_init();
    while(1){
        char *line = sf_readline("jobber> ");
        if(!line) exit(EXIT_SUCCESS);
        myCmd = parseCmd(line);
        sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
        // sigemptyset(&mask);
        // sigaddset(&mask, SIGCHLD);
        // sigprocmask(SIG_BLOCK, &mask, NULL);
        valCmd(myCmd);
        free(myCmd.cmd);
        sigprocmask(SIG_SETMASK, &prev_all, NULL);
        // sigprocmask(SIG_UNBLOCK, &mask, NULL);
    }
    exit(EXIT_SUCCESS);
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
