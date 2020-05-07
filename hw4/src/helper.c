#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "helper.h"

void child_handler(int signum){
    pid_t pid ;
    int status;
    while ((pid = waitpid(-1, &status,WNOHANG)) > 0) {
        debug("reaped: %d", pid);
        for(int i = 0; i < MAX_JOBS; i ++){
            if(job_get_pgid(i) == pid){
                debug("pid caught : %d", pid);
                if(WIFEXITED(status)){
                    JOB_STATUS jobStatus = job_get_status(i);
                    debug("jobstatus: %d", jobStatus);
                    jobList -> list[i] -> status = COMPLETED;
                    jobList -> list[i] -> job -> exit_status = status;
                    sf_job_end(i, pid, status);
                    sf_job_status_change (i, jobStatus, COMPLETED);
                    CURRENT_RUNNER --;
                    arg_clear();
                    break;
                }else {
                    JOB_STATUS jobStatus = job_get_status(i);
                    debug("jobstatus: %d", jobStatus);
                    jobList -> list[i] -> status = ABORTED;
                    jobList -> list[i] -> job -> exit_status = status;
                    sf_job_end(i, pid, status);
                    sf_job_status_change (i, jobStatus, ABORTED);
                    CURRENT_RUNNER --;
                    arg_clear();
                    break;
                }
            }
        }
    }

    for(int i = 0; i < MAX_JOBS; i ++){
        job_tree_manager(i);
    }


}


void job_print_status(int jobid){
    if(isExistingJob(jobid)){
        fprintf(stdout, "job %d ", jobid);
        if(!strcmp(job_status_names[jobList -> list[jobid] -> status],"completed")){
            fprintf(stdout, "[completed (%d)]: ",job_get_result(jobid));
        } else{
            fprintf(stdout, "[%s]: ",job_status_names[jobList -> list[jobid] -> status]);
        }
        if(isExistingJob(jobid)){

        }
        fprintf(stdout, "%s\n", job_get_taskspec(jobid));
    }else fprintf(stderr, "Error: status\n");
}


void JOBS_print_status(){
    if(PROC_ENABLED) fprintf(stdout, "Starting jobs is enabled\n");
    else fprintf(stdout, "Starting jobs is disabled\n");
    for(int i = 0; i < MAX_JOBS; i++){
        if(isExistingJob(i)){
            fprintf(stdout, "job %d ", i);
            if(!strcmp(job_status_names[jobList -> list[i] -> status],"completed")){
                fprintf(stdout, "[completed (%d)]: ",job_get_result(i));
            } else{
                fprintf(stdout, "[%s]: ",job_status_names[jobList -> list[i] -> status]);
            }
            fprintf(stdout, "%s\n", job_get_taskspec(i));
        }
    }
}


static void fd_redir(int oldfd, int newfd) {
    if(oldfd != newfd) {
        if (dup2(oldfd, newfd) != -1) close(oldfd);
        else{
            fprintf(stderr, "dup2 error");
            exit(errno);
        }
    }
}


static void arg_clear(){
    int i = 0;
    while(ARG[i]){
        ARG[i] = "";
        i ++;
    }
}


static void task_runner(TASK *tp){
    PIPELINE_LIST *pipeList = tp -> pipelines;
    while(pipeList){
        PIPELINE *line = pipeList -> first;
        char * outfile = pipeList -> first -> output_path;
        if(outfile){
            int new_fd = open(outfile, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
            fd_redir(new_fd, 1);
        }
        COMMAND_LIST *cmdList = line -> commands;
        while(cmdList){
            COMMAND *cmd = cmdList -> first;
            WORD_LIST *wrdList = cmd -> words;
            int i = 0;
            while(wrdList){
                ARG[i] = wrdList -> first;
                debug("arg[%d]: %s",i,ARG[i]);
                i ++;
                wrdList = wrdList -> rest;
            }
            cmdList = cmdList -> rest;//means cmd1|cmd2
            int status;
            if(!cmdList){
                if(fork() == 0){
                    execvp(ARG[0], ARG);
                    perror("execvp failed");
                    abort();
                    // kill(0, SIGKILL);
                }
                // wait(NULL);
                waitpid(-1, &status,0);
                if (WIFEXITED(status)) {
                   debug("exit status: %d", status);
                    STATUS = status;
                } else if (WIFSIGNALED(status)) {
                   abort();
                } /*else if (WIFSTOPPED(status)) {
                   printf("stopped by signal %d\n", WSTOPSIG(status));
                } else if (WIFCONTINUED(status)) {
                   printf("continued\n");
                }*/
            }else{
                int pipefd[2];
                if(pipe(pipefd) == -1){
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                if(fork() == 0){
                    close(pipefd[0]);//close unused read end
                    dup2(pipefd[1], 1);//dup out to pipefd[1]
                    execvp(ARG[0], ARG);
                    perror("execvp failed");
                    abort();
                    // kill(0, SIGABRT);
                }//else{
                close(pipefd[1]);//close unused write end
                dup2(pipefd[0], 0);
                // wait(NULL);
                waitpid(-1, &status,0);
                if (WIFEXITED(status)) {
                    debug("status: %d", status);
                    STATUS = status;
                } else if (WIFSIGNALED(status)) {
                   abort();
                } /*else if (WIFSTOPPED(status)) {
                   printf("stopped by signal %d\n", WSTOPSIG(status));
                } else if (WIFCONTINUED(status)) {
                   printf("continued\n");
                }*/
            }
        }
        pipeList = pipeList -> rest;
    }
}


char *substring(char *str, const char *pos, int len) {
    char *ptr;
    int i;

    ptr = (char *) malloc(len + 1);

    if (ptr == NULL) {
        fprintf(stderr, "Unable to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < len; i++) {
        *(ptr + i) = *(pos + i);
        str++;
    }

    *(ptr + i) = '\0';

    return ptr;
}



userCmd parseCmd(char *str) {
    userCmd myCmd;
    myCmd.cmd = NULL;
    myCmd.arg = NULL;
    myCmd.count = 0;
    int cmdFlag = 0;
    char *start;
    char *end;
    char *buff;
    start = str;

    while (*start) {
        /*single-quoted string with backslash escapes.*/
        if (*start == '\'') {
            end = strchr(++start, '\'');
            if (end == NULL || *end == '\0') {

                fprintf(stderr, "Unterminated single-quoted string -- %s\n", --start);
                break;
            }
            int len = end - start;
//            while (end != NULL && *end == '\'')
//                end++;
            if (len <= 0) goto skip;
            buff = substring(str, start, len);

            if (!cmdFlag) {
                cmdFlag++;
                myCmd.cmd = (char *) malloc(len + 1);
                if (myCmd.cmd != NULL) {
                    strcpy(myCmd.cmd, buff);//copy buff to cmd
                } else {
                    fprintf(stderr, "Unable to allocate memory.\n");
                    exit(EXIT_FAILURE);
                }
            } else {//if cmd flag is on
                if (!myCmd.count) {//if argument has not been assigned yet
                    myCmd.arg = (char *) malloc(len + 1);
                    if (myCmd.arg != NULL) {
                        strcpy(myCmd.arg, buff);
                    } else {
                        fprintf(stderr, "Unable to allocate memory.\n");
                        exit(EXIT_FAILURE);
                    }
                }
                myCmd.count++;
            }
            free(buff);

        } else if (*start != '\0' && *start != '\'') {
            end = start + 1;
            while (*end != '\0' && *end != ' ')
                end++;
            int len = end - start;
            buff = substring(str, start, len);
            if (!cmdFlag) {
                cmdFlag++;
                myCmd.cmd = (char *) malloc(len + 1);
                if (myCmd.cmd != NULL) {
                    strcpy(myCmd.cmd, buff);//copy buff to cmd
                } else {
                    fprintf(stderr, "Unable to allocate memory.\n");
                    exit(EXIT_FAILURE);
                }
            } else {//if cmd flag is on
                if (!myCmd.count) {//if argument has not been assigned yet
                    myCmd.arg = (char *) malloc(len + 1);
                    if (myCmd.arg != NULL) {
                        strcpy(myCmd.arg, buff);
                    } else {
                        fprintf(stderr, "Unable to allocate memory.\n");
                        exit(EXIT_FAILURE);
                    }
                }
                myCmd.count++;
            }
            free(buff);
        }

            /*Empty string.*/
        else
            end = start;

        /*Ending quotes to be skipped before continuing parsing.*/
        skip:
        if (*end == '\'')
            ++end;

        start = end;

        while (*start == ' ')
            ++start;
    }
    return myCmd;
}

void job_tree_manager(int jobid){
    pid_t masterpid;
    JOB *job = get_job(jobid);
    JOB_STATUS status = job_get_status(jobid);
    if(CURRENT_RUNNER < 4 && jobs_get_enabled() && get_occupation(jobid) == OCCUPIED && job_get_status(jobid) == WAITING){
        CURRENT_RUNNER ++;
        if((masterpid = fork()) == 0){
            sf_job_start(jobid, getpid());
            sf_job_status_change(jobid, status, RUNNING);
            setpgid(0,0);
            debug("pid: %d, pgid: %d", getpid(), getpgid(getpid()));
            task_runner(job -> task);
            exit(STATUS);/********************/
        }
        job -> pgid = masterpid;
        if(masterpid) jobList -> list[jobid] -> status = RUNNING;
    }
}


void valCmd(userCmd myCmd) {
    debug("main pid: %d", getpid());
    if (myCmd.cmd == NULL) return;
    if (!strcmp(myCmd.cmd, "help")) {
        fprintf(stdout, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
                "Available commands:", "help (0 args) Print this help message",
                "quit (0 args) Quit the program", "enable (0 args) Allow jobs to start",
                "disable (0 args) Prevent jobs from starting", "spool (1 args) Spool a new job",
                "pause (1 args) Pause a running job", "resume (1 args) Resume a paused job",
                "cancel (1 args) Cancel an unfinished job", "expunge (1 args) Expunge a finished job",
                "status (1 args) Print the status of a job", "jobs (0 args) Print the status of all jobs"
        );
    } else if (!strcmp(myCmd.cmd, "quit")) {
        //cancel all jobs that have not terminated
        //expunge all jobs from table
        jobs_fini();
        exit(EXIT_SUCCESS);
    } else if (!strcmp(myCmd.cmd, "jobs")) {
        if (myCmd.count == 0) {
            //print all
            JOBS_print_status();
        } else {
            fprintf(stderr, "Wrong number of args (given: %d, required: 0) for command '%s'", myCmd.count, myCmd.cmd);
        }
    } else if (!strcmp(myCmd.cmd, "enable")) {
        if (myCmd.count == 0) {
            jobs_set_enabled(ENABLED);
        } else {
            fprintf(stderr, "Wrong number of args (given: %d, required: 0) for command '%s'", myCmd.count, myCmd.cmd);
        }
    } else if (!strcmp(myCmd.cmd, "disable")) {
        if (myCmd.count == 0) {
            jobs_set_enabled(DISABLED);
        } else {
            fprintf(stderr, "Wrong number of args (given: %d, required: 0) for command '%s'\n", myCmd.count, myCmd.cmd);
        }
    } else if (!strcmp(myCmd.cmd, "status")) {
        if (myCmd.count == 1) {
            int jobid = *myCmd.arg - '0';
            job_print_status(jobid);
        } else {
            fprintf(stderr, "Wrong number of args (given: %d, required: 1) for command '%s'\n", myCmd.count, myCmd.cmd);
        }
    } else if (!strcmp(myCmd.cmd, "pause")) {
        if (myCmd.count == 1) {
            int jobid = *myCmd.arg - '0';
            job_pause(jobid);
        } else {
            fprintf(stderr, "Wrong number of args (given: %d, required: 1) for command '%s'\n", myCmd.count, myCmd.cmd);
        }
    } else if (!strcmp(myCmd.cmd, "resume")) {
        if (myCmd.count == 1) {
            int jobid = *myCmd.arg - '0';
            job_resume(jobid);
        } else {
            fprintf(stderr, "Wrong number of args (given: %d, required: 1) for command '%s'\n", myCmd.count, myCmd.cmd);
        }
    } else if (!strcmp(myCmd.cmd, "cancel")) {
        if (myCmd.count == 1) {
            int jobid = *myCmd.arg - '0';
            job_cancel(jobid);
        } else {
            fprintf(stderr, "Wrong number of args (given: %d, required: 1) for command '%s'\n", myCmd.count, myCmd.cmd);
        }
    } else if (!strcmp(myCmd.cmd, "expunge")) {
        if (myCmd.count == 1) {
            int jobid = *myCmd.arg - '0';
            job_expunge(jobid);
        } else {
            fprintf(stderr, "Wrong number of args (given: %d, required: 1) for command '%s'\n", myCmd.count, myCmd.cmd);
        }
    } else if (!strcmp(myCmd.cmd, "spool")) {
        if (myCmd.count == 1) {
            int jobid = job_create(myCmd.arg);
            if(isExistingJob(jobid)) job_tree_manager(jobid);
        } else {
            fprintf(stderr, "Wrong number of args (given: %d, required: 1) for command '%s'\n", myCmd.count, myCmd.cmd);
        }
    } else {
        fprintf(stderr, "Unrecognized command: %s\n", myCmd.cmd);
    }
}


JOB_OCCUPATION get_occupation(int jobid){
    if(jobList -> list[jobid] != NULL) return jobList -> list[jobid] -> occupation;
    return -1;
}

JOB *get_job(int jobid){
    return jobList -> list[jobid] -> job;
}

int isExistingJob(int jobid){
    if((jobid >= 0 && jobid < MAX_JOBS) && (jobList -> list[jobid] -> job != NULL)
        && (jobList -> list[jobid] -> occupation == OCCUPIED)) {
        return 1;
    }
    return 0;
}