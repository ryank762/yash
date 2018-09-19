/* yash.c
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

#include "runCmd.h"
#include "jobs.h"

void reInitTokenTable(char** tokenTable);

char* currentCommand;
char* TokenTable[1001];
int TokenCount = 0;

int main() {
    char* command;
    char* token;
    int pipeIdx;

    init_ListHead();
    signal(SIGINT, SIGINT_Handler);
    signal(SIGTSTP, SIGTSTP_Handler);
    signal(SIGCHLD, SIGCHLD_Handler);


    while (1) {
        pipeIdx = -1;
        command = readline("# ");
        if (command == NULL) {
            printf("\n");
            exit(1);
        }
        currentCommand = (char*) malloc((strlen(command) + 1) * sizeof(char));
        strcpy(currentCommand, command);
        token = strtok(command, " ");
        TokenTable[0] = token;
        for (TokenCount = 0; token != NULL; TokenCount++) {
            if (strcmp(token, "|") == 0) {
                pipeIdx = TokenCount;
            }
            token = strtok(NULL, " ");
            TokenTable[TokenCount + 1] = token;
        }

        if (strcmp(TokenTable[0], "jobs") == 0 && TokenTable[1] == NULL) {
            printJobTable();
        } else {
            printDoneJobs();
            if (strcmp(TokenTable[0], "fg") == 0 && TokenTable[1] == NULL) {
                sendToForeground();
            } else {
                if (strcmp(TokenTable[0], "bg") == 0 && TokenTable[1] == NULL) {
                    sendToBackground();
                } else {
                    if (pipeIdx < 0) {
                        execCmd1(0, TokenCount);
                    } else {
                        execCmd2(pipeIdx);
                    }
                }
            }
        }
        reInitTokenTable(TokenTable);
        free(command);
    }
    return 0;
}

void reInitTokenTable(char** tokenTable) {
    for (TokenCount; TokenCount > 0; TokenCount--) {
        tokenTable[TokenCount-1] = NULL;
    }
}
