/* jobs.c
 * Ryan Kim
 * jk39938
 * EE461S Yerraballi
 * 9/18/2018
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "yash.h"
#include "runCmd.h"

enum status_t {
    RUNNING, STOPPED, DONE
};

typedef struct JobList {
    int jobNum;
    pid_t pid1;
    pid_t pid2;
    bool isBackground;
    bool isProcessGroup;
    enum status_t status;
    int doneCount;
    char* command;
    struct JobList* next;
} JobList;

typedef struct JobListHead {
    JobList* newestJob;
} JobListHead;

JobListHead ListHead;

void init_ListHead() {
     ListHead.newestJob = NULL;
}

void addJob(pid_t pid1, pid_t pid2, bool isBackground, bool isProcessGroup, char* command) {
    JobList* newJob = (JobList*) malloc(sizeof(JobList));
    newJob->jobNum = (ListHead.newestJob == NULL) ? 1 : ListHead.newestJob->jobNum + 1;
    newJob->pid1 = pid1;
    newJob->pid2 = (isProcessGroup) ? pid2 : -1;
    newJob->isBackground = isBackground;
    newJob->isProcessGroup = isProcessGroup;
    newJob->status = RUNNING;
    newJob->doneCount = (isProcessGroup) ? 0 : -1;
    newJob->next = ListHead.newestJob;
    if (currentCommand[strlen(currentCommand)-1] != '&') {
        newJob->command = (char *) malloc(strlen(command) + 1 * sizeof(char));
        strcpy(newJob->command, currentCommand);
    } else {
        newJob->command = (char *) calloc(strlen(command) - 1, sizeof(char));
        strncpy(newJob->command, currentCommand, strlen(currentCommand) - 2);
    }

    ListHead.newestJob = newJob;
    return;
}

int deleteJob(pid_t pid) {      // return 0 on success, -1 on fail
    JobList* current = ListHead.newestJob;
    JobList* previous = NULL;
    while (current != NULL && current->pid1 != pid && current->pid2 != pid) {
        previous = current;
        current = current->next;
    }
    if (current == NULL) {
        return -1;
    }
    if (current == ListHead.newestJob) {
        ListHead.newestJob = current->next;
    } else {
        previous->next = current->next;
    }
    free(current->command);
    free(current);

    return 0;
}

JobList* findJobByStatus(enum status_t STATUS) {
    JobList* current = ListHead.newestJob;
    while (current != NULL && current->status != STATUS) {
        current = current->next;
    }
    if (current == NULL) {
        return NULL;
    }
    return current;
}

JobList* findJobByPID(pid_t pid) {
    JobList* current = ListHead.newestJob;
    while (current != NULL && current->pid1 != pid && current->pid2 != pid) {
        current = current->next;
    }
    if (current == NULL) {
        return NULL;
    }
    return current;
}


void sendToForeground() {
    pid_t cpid;
    int status;
    siginfo_t siginfo;
    JobList* current = ListHead.newestJob;
    if (current != NULL) {
        current->status = RUNNING;
        current->isBackground = false;
        printf("%s\n", current->command);
        //printf("sending SIGCONT to process with pid: %d\n", current->pid);
        if (current->isProcessGroup) {
            if (current->pid1 != -1) {
                kill(current->pid1, SIGCONT);
            }
            if (current->pid2 != -1) {
                kill(current->pid2, SIGCONT);
            }
            if (current->pid1 != -1) {
                waitid(P_PID, current->pid1, &siginfo, WEXITED | WSTOPPED);
            }
            if (current->pid2 != -1) {
                waitid(P_PID, current->pid2, &siginfo, WEXITED | WSTOPPED);
            }
            //pause();
        } else {
            kill(current->pid1, SIGCONT);
            waitid(P_PID, current->pid1, &siginfo, WEXITED | WSTOPPED);
            //pause();
        }
        //cpid = waitpid(-1, &status, WNOHANG);
        //pause();
    }
}

void sendToBackground() {
    JobList* current = findJobByStatus(STOPPED);
    if (current != NULL) {
        current->status = RUNNING;
        current->isBackground = true;
        printf("%s &\n", current->command);
        if (current->isProcessGroup) {
            if (current->pid1 != -1) {
                kill(current->pid1, SIGCONT);
            }
            if (current->pid2 != -1) {
                kill(current->pid2, SIGCONT);
            }
        } else {
            kill(current->pid1, SIGCONT);
        }
    }
}

int convert(bool isFirst) {
    return (isFirst) ? '+' : '-';
}

bool isRecentJob(JobList* job) {
    if (job == NULL) return false;
    return (job == ListHead.newestJob) ? true : false;
}

void printJobSingle(JobList* job) {
    switch (job->status) {
        case RUNNING : {
            printf("[%d]%c\t\tRunning\t\t%s", job->jobNum, convert(isRecentJob(job)), job->command);
            break;
        }
        case STOPPED : {
            printf("[%d]%c\t\tStopped\t\t%s", job->jobNum, convert(isRecentJob(job)), job->command);
            break;
        }
        case DONE : {
            printf("[%d]%c\t\tDone\t\t%s", job->jobNum, convert(isRecentJob(job)), job->command);
            break;
        }
    }
    if (job->isBackground) {
        printf(" &\n");
    } else {
        printf("\n");
    }
}

void cleanUpDoneJobs() {
    JobList* next;
    JobList* current = ListHead.newestJob;
    while (current != NULL) {
        next = current->next;
        if (current->status == DONE) {
            deleteJob(current->pid1);
        }
        current = next;
    }
}

void printDoneJobs() {
    JobList* next;
    JobList* current = ListHead.newestJob;
    while (current != NULL) {
        next = current->next;
        if (current->status == DONE) {
            if (current->isBackground == true) {
                printJobSingle(current);
            }
            deleteJob(current->pid1);
        }
        current = next;
    }
}

void printJobRec(JobList* job, bool isFirst) {
    if (job == NULL) {
        return;
    }
    printJobRec(job->next, false);

    printJobSingle(job);
}

void printJobTable() {
    if (ListHead.newestJob != NULL) {
        printJobRec(ListHead.newestJob, true);
    }
    cleanUpDoneJobs();
}

void SIGINT_Handler(int signal) {
    //JobList* current = findJobByStatus(RUNNING);
    JobList* current = ListHead.newestJob;
    if (current == NULL) {
        return;
    }
    if (current->isBackground == false) {
        if (current->isProcessGroup == true) {
            if (current->pid1 != -1) {
                kill(current->pid1, SIGKILL);
            }
            if (current->pid2 != -1) {
                kill(current->pid2, SIGKILL);
            }
            if (deleteJob(current->pid1) == -1) deleteJob(current->pid2);
        } else {
            kill(current->pid1, SIGKILL);
            deleteJob(current->pid1);
        }
    }
}

void SIGTSTP_Handler(int signal) {
    //printf("caught SIGTSTP\n");
    JobList* current = findJobByStatus(RUNNING);
    if (current == NULL) {
        return;
    }
    /*
    printf("command : %s\n", current->command);
    printf("isBackground: %d\n", current->isBackground);
     */
    if (current->isBackground == false) {
        /*
        printf("yash cpid: %d\n", getpid());
        printf("child cpid: %d\n", current->pid);
        printf("in a process group? %d\n", current->isProcessGroup);
         */
        current->status = STOPPED;
        printJobTable();
        if (current->isProcessGroup == true) {
            if (current->pid1 != -1) {
                kill(current->pid1, SIGTSTP);
            }
            if (current->pid2 != -1) {
                kill(current->pid2, SIGTSTP);
            }
        } else {
            kill(current->pid1, SIGTSTP);
        }
    }
}

void SIGCHLD_Handler(int signal) {
    int status;
    pid_t cpid = waitpid(-1, &status, WNOHANG);
    //printf("caught SIGCHLD %d\n", cpid);
    JobList* current = findJobByPID(cpid);
    if (current != NULL) {
        if (current->status == RUNNING && current->isBackground == true) {
            if (current->isProcessGroup) {
                if (current->doneCount == 1) {
                    current->status = DONE;
                }
                current->doneCount++;
            } else {
                current->status = DONE;
            }
        }
        if (current->status == RUNNING && current->isBackground == false) {
            if (current->isProcessGroup == true) {
                if (current->doneCount == 0) {
                    if (cpid == current->pid1) {
                        current->doneCount++;
                        current->pid1 = -1;
                    }
                    if (cpid == current->pid2) {
                        current->doneCount++;
                        current->pid2 = -1;
                    }
                } else {
                    if(deleteJob(current->pid1) == -1) deleteJob(current->pid2);
                }
            } else {
                if (deleteJob(current->pid1) == -1) deleteJob(current->pid2);
            }
        }
    }
}
