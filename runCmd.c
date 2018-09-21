/* runCmd.c
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
#include <sys/types.h>
#include <sys/wait.h>

#include "yash.h"
#include "jobs.h"

void execCmd(int startIdx, int endIdx) {
    int stdinIdx = -1;
    int stdoutIdx = -1;
    int stderrIdx = -1;

    for (int i = startIdx; i < endIdx; i++) {
        if (strcmp(TokenTable[i], "<") == 0) {
            stdinIdx = i;
        }
        if (strcmp(TokenTable[i], ">") == 0) {
            stdoutIdx = i;
        }
        if (strcmp(TokenTable[i], "2>") == 0) {
            stderrIdx = i;
        }
    }

    if (stdinIdx != -1) {
        int fd = open(TokenTable[stdinIdx + 1], O_RDONLY,
                      S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (stdoutIdx != -1) {
        int fd = open(TokenTable[stdoutIdx + 1], O_WRONLY|O_CREAT|O_TRUNC,
                      S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    if (stderrIdx != -1) {
        int fd = open(TokenTable[stderrIdx + 1], O_WRONLY|O_CREAT|O_TRUNC,
                      S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
        dup2(fd, STDERR_FILENO);
        close(fd);
    }

    int i, j, execParamCount = 0;
    for (i = startIdx; i < endIdx; i++) {
        if ((strcmp(TokenTable[i], ">") == 0) || (strcmp(TokenTable[i], "<") == 0)
            || (strcmp(TokenTable[i], "2>") == 0) || (strcmp(TokenTable[i], "&") == 0)) {
            i++;
            continue;
        }
        execParamCount++;
    }

    i = 0;
    j = startIdx;
    char* execParamArray[execParamCount + 1];

    while (i < execParamCount) {
        if ((strcmp(TokenTable[j], ">") == 0) || (strcmp(TokenTable[j], "<") == 0)
            || (strcmp(TokenTable[j], "2>") == 0) || (strcmp(TokenTable[j], "&") == 0)) {
            j += 2;
            continue;
        }
        execParamArray[i] = TokenTable[j];
        i++; j++;
    }
    execParamArray[i] = NULL;

    execvp(execParamArray[0], execParamArray);
}

void execCmd1(int startIdx, int endIdx) {   // no pipes
    pid_t cpid;

    cpid = fork();
    if (cpid == 0) {               // child
        execCmd(startIdx, TokenCount);

    } else {                        // parent
        if (strcmp(TokenTable[endIdx-1], "&") != 0) {
            /*
            printf("parent pid: %d\n", getpid());                   // debugging
            printf("child pid: %d\n", cpid);                        // debugging
             */
            addJob(cpid, -1, false, false, currentCommand);
            pause();
        } else {
            addJob(cpid, -1, true, false, currentCommand);
        }
    }
}

void execCmd2(int pipeIdx) {                // contains pipes
    pid_t cpid1, cpid2;
    int pipefd[2];

    pipe(pipefd);

    cpid1 = fork();
    setpgid(cpid1, cpid1);
    if (cpid1 == 0) {                   // child 1
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execCmd(0, pipeIdx);

    } else {                            // parent and child 2
        cpid2 = fork();
        setpgid(cpid2, cpid1);
        if (cpid2 == 0) {               // child 2
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            execCmd(pipeIdx+1, TokenCount);

        } else {                        // parent
            close(pipefd[0]);
            close(pipefd[1]);
            if (strcmp(TokenTable[TokenCount-1], "&") != 0) {
                addJob(cpid1, cpid2, false, true, currentCommand);
                pause();
                pause();
            } else {
                addJob(cpid1, cpid2, true, true, currentCommand);
            }
       }
    }
}
