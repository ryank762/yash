/* jobs.h
 * Ryan Kim
 * jk39938
 * EE461S Yerraballi
 * 9/18/2018
 */
#include <stdbool.h>
#include <sys/types.h>

enum status_t {                     // data type that enumerates process states
    RUNNING, STOPPED, DONE
};

typedef struct JobList {            // reversed linked list data structure (most recent is head)
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

extern JobListHead ListHead;        // global pointer to the head of reversed linked list

void init_ListHead();               // sets the ListHead pointer

void addJob(pid_t pid1, pid_t pid2, bool isBackground, bool isProcessGroup, char* command);
                                    // adds a job to the linked list data structure
void deleteJob(pid_t pid);          // deletes a job from the linked list data structure
JobList* findJob(pid_t pid);        // finds a job from the linked list data structure

void sendToForeground();            // sends a process to the foreground of shell when "fg" is entered
void sendToBackground();            // sends a process to the background of shell when "bg" is entered
void printJobTable();               // prints the job table when "jobs" is entered in the shell
void cleanUpDoneJobs();             // removes jobs in the DONE state
void printDoneJobs();               // prints only jobs that are finished (runs when "jobs" is not entered in cmd line)

void SIGINT_Handler(int signal);    // handler that catches SIGINTs (sends SIGKILL to children processes)
void SIGTSTP_Handler(int signal);   // handler that catches SIGTSTPs (sends SIGTSTP to children processes)
void SIGCHLD_Handler(int signal);   // handler that catches SIGCHLDs (updates jobs linked list data structure)
