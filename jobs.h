/* jobs.h
 * Ryan Kim
 * jk39938
 * EE461S Yerraballi
 * 9/18/2018
 */
#include <stdbool.h>
#include <sys/types.h>

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
    char* command;
    struct JobList* next;
} JobList;

typedef struct JobListHead {
    JobList* newestJob;
} JobListHead;

extern JobListHead ListHead;

void init_ListHead();

void addJob(pid_t pid1, pid_t pid2, bool isBackground, bool isProcessGroup, char* command);
void deleteJob(pid_t pid);
JobList* findJob(pid_t pid);

void sendToForeground();
void sendToBackground();
void printJobTable();
void cleanUpDoneJobs();
void printDoneJobs();

void SIGINT_Handler(int signal);
void SIGTSTP_Handler(int signal);
void SIGCHLD_Handler(int signal);
