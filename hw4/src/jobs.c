/*
 * Job manager for "jobber".
 */
#include <stdlib.h>
#include "helper.h"
// #include "jobber.h"
// #include "task.h"

int jobs_init(void) {
    PROC_ENABLED = 0;
    CURRENT_RUNNER = 0;
    jobList = (job_list*)malloc(sizeof(job_list));
    if(jobList != NULL){
        jobList -> jobs = 0;
        for(int i = 0; i < MAX_JOBS; i++){
            jobList -> list[i] = (job_space *)malloc(sizeof(job_space));
            jobList -> list[i] -> jobid = i;//jobid to be fixed
            jobList -> list[i] -> status = NEW;
            jobList -> list[i] -> occupation = VACANT;//all job_space to be vacant
            jobList -> list[i] -> job = (JOB*)malloc(sizeof(JOB));
        }
        return 0;
    }
    return -1;
}

void jobs_fini(void) {
    /*free the joblist*/
    for(int i = 0; i < MAX_JOBS; i++){
        JOB_STATUS status = job_get_status(i);
        if(isExistingJob(i)&&
        (status == WAITING || status == RUNNING || status == PAUSED)) job_cancel(i);
        if(isExistingJob(i))job_expunge(i);
        free(jobList -> list[i] -> job);
        free(jobList -> list[i]);
    }
    free(jobList);
}

int jobs_set_enabled(int val) {
    //set a public flag to get each time when a new job is created
    //enable all jobs when called
    PROC_ENABLED = val;
    if(PROC_ENABLED){
        for(int i = 0; i < MAX_JOBS; i ++){
            job_tree_manager(i);
        }
    }
    return 0;
}

int jobs_get_enabled() {
    //check flag when new job is created
    return PROC_ENABLED;
}

int job_create(char *command) {
    if (jobList -> jobs < MAX_JOBS) {
        for(int i = 0; i < MAX_JOBS; i++){
            if(jobList -> list[i] -> occupation == VACANT){
                JOB_STATUS status = jobList -> list[i] -> status;
                /*set job_space to be occupied*/
                jobList -> list[i] -> occupation = OCCUPIED;
                jobList -> list[i] -> description = command;
                JOB *newJob = jobList -> list[i] -> job;
                newJob -> task = parse_task(&command);
                jobList -> jobs ++;
                if(jobList -> list[i] -> job == NULL) debug("null job\n");
                int jobid = jobList -> list[i] -> jobid;
                jobList -> list[i] -> status = WAITING;
                sf_job_create(jobid);
                sf_job_status_change(jobid, status, WAITING);
                return jobid;
            }
        }

    }else{
        fprintf(stderr, "Error: spool\n");
        return -1;
    }
    return -1;
}

int job_expunge(int jobid) {
    JOB_STATUS status = job_get_status(jobid);
    if(isExistingJob(jobid)&&
        (status == COMPLETED || status == ABORTED)){
        jobList -> list[jobid] -> occupation = VACANT;
        jobList -> list[jobid] -> status = NEW;
        free_task(jobList -> list[jobid] -> job -> task);
        free(jobList -> list[jobid] -> description);
        jobList -> jobs --;
        sf_job_expunge(jobid);
        return 0;
    }else{
        fprintf(stderr, "Error: expunge\n");
        return -1;
    }
    return -1;
}


/* Note that this function must appropriately mask signals in order to avoid a race
 * between the querying of the job status and the subsequent setting of the status
 * to RUNNING and the sending of the SIGCONT signal.*/
int job_cancel(int jobid) {
    JOB_STATUS status = job_get_status(jobid);
    if(isExistingJob(jobid) &&
        (status == WAITING || status == RUNNING || status == PAUSED)){
        if(status == RUNNING || status == PAUSED){
            if(kill(-job_get_pgid(jobid), SIGKILL) == 0){
                jobList -> list[jobid] -> status = CANCELED;
                sf_job_status_change(jobid, status, CANCELED);
                return 0;
            }
        }else{
            jobList -> list[jobid] -> status = ABORTED;
            sf_job_status_change(jobid, status, ABORTED);
            return 0;
        }
    }
    fprintf(stderr, "Error: cancel\n");
    return -1;
}


/* Note that this function must appropriately mask signals in order to avoid a race
 * between the querying of the job status and the subsequent setting of the status
 * to RUNNING and the sending of the SIGCONT signal.*/
int job_pause(int jobid) {
    JOB_STATUS status = job_get_status(jobid);
    if(isExistingJob(jobid) && (status == RUNNING)){
        if(kill(-job_get_pgid(jobid), SIGSTOP) == 0){
            jobList -> list[jobid] -> status = PAUSED;
            sf_job_status_change(jobid, status, PAUSED);
            return 0;
        }
    }
    fprintf(stderr, "Error: pause\n");
    return -1;
}


/*
 * @brief  Resume a paused job.
 * @details  This function takes a job ID as a parameter and, if that job is in the
 * PAUSED state, then it is set to the RUNNING state and a SIGCONT signal is sent
 * to the process group of the job's runner process in order to cause the job to
 * continue execution.
 * Note that this function must appropriately mask signals in order to avoid a race
 * between the querying of the job status and the subsequent setting of the status
 * to RUNNING and the sending of the SIGCONT signal.*/
int job_resume(int jobid) {
    JOB_STATUS status = job_get_status(jobid);
    if(isExistingJob(jobid) && (status == PAUSED)){
        if(kill(-job_get_pgid(jobid), SIGCONT) == 0){
            jobList -> list[jobid] -> status = RUNNING;
            sf_job_status_change(jobid, status, RUNNING);
            return 0;
        }
    }
    fprintf(stderr, "Error: resume\n");
    return -1;
}

int job_get_pgid(int jobid) {
    JOB_STATUS status = job_get_status(jobid);
    if(isExistingJob(jobid) &&
        (status == RUNNING || status == PAUSED || status == CANCELED)) return jobList -> list[jobid] -> job -> pgid;
    return -1;
}

JOB_STATUS job_get_status(int jobid) {
    if(isExistingJob(jobid)) return jobList -> list[jobid] -> status;
    return -1;
}

int job_get_result(int jobid) {
    if(isExistingJob(jobid) && (job_get_status(jobid) == COMPLETED)) return jobList -> list[jobid] -> job -> exit_status;
    return -1;
}

int job_was_canceled(int jobid) {
    JOB_STATUS status = job_get_status(jobid);
    if(isExistingJob(jobid) && (status == ABORTED || status== CANCELED)) return 1;
    else return 0;
}

char *job_get_taskspec(int jobid) {
    if(isExistingJob(jobid)) {
        return jobList -> list[jobid] -> description;
    }else return NULL;
}

