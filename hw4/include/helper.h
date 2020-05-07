#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "jobber.h"
#include "task.h"

#define ENABLED 1

#define DISABLED 0

#define MAX_STR 255

int STATUS;

int PROC_ENABLED;

int CURRENT_RUNNER;

char *ARG[MAX_STR];

typedef enum{
    VACANT,
    OCCUPIED
}JOB_OCCUPATION;


typedef struct userCmd {
    char *cmd;
    char *arg;
    int count;
} userCmd;


typedef struct JOB {
    pid_t pgid;
    TASK *task;
    int exit_status;
} JOB;


typedef struct job_space {
    int jobid;
    char *description;
    JOB_OCCUPATION occupation;
    JOB_STATUS status;
    JOB *job;
} job_space;


typedef struct job_list {
    int jobs;
    struct job_space *list[MAX_JOBS];
} job_list;

userCmd myCmd;

job_list *jobList;

void child_handler(int signum);

char *substring(char *str, const char *pos, int len);

userCmd parseCmd(char *str);

void valCmd(userCmd myCmd);

JOB_OCCUPATION get_occupation(int jobid);

JOB *get_job(int jobid);

int isExistingJob(int jobid);

static void arg_clear();

static void fd_redir(int oldfd, int newfd);

static void task_runner(TASK *tp);

void job_tree_manager(int jobid);

void job_print_status(int jobid);

void JOBS_print_status();