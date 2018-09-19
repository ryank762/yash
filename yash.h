/* yash.h
 * Ryan Kim
 * jk39938
 * EE461S Yerraballi
 * 9/16/2018
 */

extern char* TokenTable[1000];
                                                // global array containing pointers to tokens of input command
extern int TokenCount;
                                                // global int containing numbers of tokens in TokenTable
extern char* currentCommand;
                                                // deep copy of command once read
