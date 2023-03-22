//P2-SSOO-22/23

// MSH main file
// Write your msh source code here

//#include "parser.h"
#include <stddef.h>			/* NULL */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#define MAX_COMMANDS 8


// files in case of redirection
char filev[3][64];

//to store the execvp second parameter
char *argv_execvp[8];
int Acc = 0;


void siginthandler(int param)
{
    printf("****  Exiting MSH **** \n");
    //signal(SIGINT, siginthandler);
    exit(0);
}


/* Timer */
pthread_t timer_thread;
unsigned long  mytime = 0;

void* timer_run ( )
{
    while (1)
    {
        usleep(1000);
        mytime++;
    }
}

/**
 * Get the command with its parameters for execvp
 * Execute this instruction before run an execvp to obtain the complete command
 * @param argvv
 * @param num_command
 * @return
 */
void getCompleteCommand(char*** argvv, int num_command) {
    //reset first
    for(int j = 0; j < 8; j++)
        argv_execvp[j] = NULL;

    int i = 0;
    for ( i = 0; argvv[num_command][i] != NULL; i++)
        argv_execvp[i] = argvv[num_command][i];
}


/**
 * Main sheell  Loop
 */
int main(int argc, char* argv[]) {
    int fd_open;
    int dupfd_open;
    int fd_read;
    int dupfd;
    /**** Do not delete this code.****/
    int end = 0;
    int executed_cmd_lines = -1;
    char *cmd_line = NULL;
    char *cmd_lines[10];

    if (!isatty(STDIN_FILENO)) {
        cmd_line = (char *) malloc(100);
        while (scanf(" %[^\n]", cmd_line) != EOF) {
            if (strlen(cmd_line) <= 0) return 0;
            cmd_lines[end] = (char *) malloc(strlen(cmd_line) + 1);
            strcpy(cmd_lines[end], cmd_line);
            end++;
            fflush(stdin);
            fflush(stdout);
        }
    }

    pthread_create(&timer_thread, NULL, timer_run, NULL);

    /*********************************/

    char ***argvv = NULL;
    int num_commands;


    while (1) {
        int status = 0;
        int command_counter = 0;
        int in_background = 0;
        signal(SIGINT, siginthandler);

        // Prompt
        write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

        // Get command
        //********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
        executed_cmd_lines++;
        if (end != 0 && executed_cmd_lines < end) {
            command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
        } else if (end != 0 && executed_cmd_lines == end) {
            return 0;
        } else {
            command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
        }
        //************************************************************************************************


        /************************ STUDENTS CODE ********************************/

        // Following the recommendations of the PDF
        if (command_counter > 0) {
            if (command_counter > MAX_COMMANDS) {
                printf("Too many commands. Maximum allowed: %d\n", MAX_COMMANDS);
            } else {
                //getCompleteCommand(argvv, 0);
                if (strcmp(argvv[0][0], "exit") == 0) {
                    printf("exiting...\n");
                    while (1) {
                        exit(0);
                    }
                    //exit(0);
                } else if (strcmp(argvv[0][0], "mytime") == 0) {
                    // Expressing the time from miliseconds to format HH:MM:SS
                    int hours = mytime / 3600000;
                    int minutes = (mytime % 3600000) / 60000;
                    int seconds = ((mytime % 3600000) % 60000) / 1000;
                    printf("mytime: %02d:%02d:%02d\n", hours, minutes, seconds);
                    //printf("mytime: %lu\n", mytime);

                } else if (strcmp(argvv[0][0], "mycalc") == 0) {
                    if ((argvv[0][1] == NULL) || (argvv[0][2] == NULL) || (argvv[0][3] == NULL) ||
                        (argvv[0][4] != NULL)) {
                        printf("[ERROR] The structure of the command is mycalc <operand_1> <add/mul/div> <operand_2>\n");
                    } else {
                        // 1. Get the first number
                        int first_number = atoi(argvv[0][1]);
                        // 2. Get the second number
                        int second_number = atoi(argvv[0][3]);
                        //printf("result: %i", first_number+second_number);
                        // 3. Get the operator + result + print
                        char operator_str[3];
                        strcpy(operator_str, argvv[0][2]);
                        if (strcmp(operator_str, "add") == 0) {
                            int result = first_number + second_number;
                            Acc += result;
                            printf("[OK] %i + %i = %i; Acc %i\n", first_number, second_number, result, Acc);
                        } else if (strcmp(operator_str, "mul") == 0) {
                            int result = first_number * second_number;
                            printf("[OK] %i * %i = %i\n", first_number, second_number, result);
                        } else if (strcmp(operator_str, "div") == 0) {
                            int result = first_number / second_number;
                            printf("[OK] %i / %i = %i; Reminder %d\n", first_number, second_number, result,
                                   first_number % second_number);
                        } else {
                            printf("[ERROR] The structure of the command is mycalc <operand_1> <add/mul/div> <operand_2>\n");
                        }
                    }
                } else if (strcmp(argvv[0][0], "cd") == 0) {
                    //printf("changing directory...\n");
                    if (argvv[0][1] == NULL) {
                        chdir(getenv("HOME"));
                    } else {
                        chdir(argvv[0][1]);
                    }

                } else {
                    // To do the redirections


                    if (command_counter == 1) {
                        // ------------------ 1 command -----------------------------------------------------------
                        // print_command(argvv, filev, in_background);
                        //printf("num_commands: %d", command_counter);
                        // Only 1 command works here, no pipes

                        /*
                        if (filev[1] != NULL) {
                            int fd = open(filev[1], O_WRONLY | O_CREAT |O_TRUNC, 0644);
                            if (fd == -1) {
                                perror("open");
                                exit(1);
                            }
                            close(STDOUT_FILENO); // close(1);
                            if (dup(fd) == -1) {
                                perror("dup");
                                exit(1);
                            }
                            close(fd);
                        }
                        */

                        // 2. Child process: execute the command:
                        pid_t pid = fork();
                        if (pid == 0) {

                            getCompleteCommand(argvv, 0);
                            execvp(argv_execvp[0], argv_execvp);

                        } else if (pid > 0) {
                            // 3. Parent process: wait for the child to finish:
                            if (in_background == 0)
                                waitpid(pid, &status, 0);
                        }
                    }
                    if (command_counter == 2) {
                        // ------------------ 2 commands -----------------------------------------------------------
                        int fd[2];
                        if (pipe(fd) == -1) {
                            perror("pipe");
                            exit(EXIT_FAILURE);
                        }

                        pid_t pid_ls, pid_wc;
                        pid_ls = fork();
                        if (pid_ls == -1) {
                            perror("fork");
                            exit(EXIT_FAILURE);
                        } else if (pid_ls == 0) {

                            close(fd[0]);
                            close(STDOUT_FILENO); // close(1);
                            dup(fd[1]);
                            close(fd[1]);
                            getCompleteCommand(argvv, 0);
                            execvp(argv_execvp[0], argv_execvp);
                            perror("execvp");
                            exit(EXIT_FAILURE);
                        }

                        pid_wc = fork();
                        if (pid_wc == -1) {
                            perror("fork");
                            exit(EXIT_FAILURE);
                        } else if (pid_wc == 0) {// Hijo
                            close(fd[1]);
                            close(STDIN_FILENO); // close(0);

                            dup(fd[0]);
                            close(fd[0]);
                            getCompleteCommand(argvv, 1);
                            execvp(argv_execvp[0], argv_execvp);
                            perror("execvp");
                            exit(EXIT_FAILURE);
                        }
                        close(fd[0]);
                        close(fd[1]);
                        if (in_background == 0) {
                            //wait(NULL);
                            waitpid(pid_ls, &status, 0);
                            waitpid(pid_wc, &status, 0);
                        }
                    } else if (command_counter == 3) {
                        // ------------------ 3 commands -----------------------------------------------------------
                        //printf("NOUP, 3 commands not supported yet\n");

                        int p1[2], p2[2];
                        if ((pipe(p1) == -1) || (pipe(p2) == -1)) {
                            perror("pipe");
                            exit(EXIT_FAILURE);
                        }

                        pid_t pid_ls = fork();
                        if (pid_ls == -1) {
                            perror("fork");
                            exit(EXIT_FAILURE);
                        } else if (pid_ls == 0) {
                            //perror("Entro en el hijo del primero");
                            // Redirection
                            close(STDOUT_FILENO);
                            dup(p1[1]);

                            // Specter Closer
                            close(p1[1]);
                            close(p1[0]);

                            // Execution
                            getCompleteCommand(argvv, 0);
                            execvp(argv_execvp[0], argv_execvp);
                            perror("execvp");
                            exit(EXIT_FAILURE);

                        } else {
                            close(p1[1]);
                        }

                        pid_t pid_grep = fork();
                        if (pid_grep == -1) {
                            perror("fork");
                            exit(EXIT_FAILURE);
                        } else if (pid_grep == 0) {
                            //perror("Entro en el hijo del segundo");
                            // Redirect input
                            close(STDIN_FILENO); // close(0);
                            dup(p1[0]);

                            // Close
                            close(p1[0]);
                            close(p1[1]);

                            // Redirect output
                            close(STDOUT_FILENO);
                            dup(p2[1]);

                            // Close
                            close(p1[0]); // Input
                            close(p2[1]);
                            close(p2[0]);

                            // Execution
                            getCompleteCommand(argvv, 1);
                            execvp(argv_execvp[0], argv_execvp);
                            perror("execvp");
                            exit(EXIT_FAILURE);
                        } else {
                            close(p1[0]);
                            close(p2[1]);
                        }

                        pid_t pid_wc = fork();
                        if (pid_wc == -1) {
                            perror("fork");
                            exit(EXIT_FAILURE);
                        } else if (pid_wc == 0) {
                            //perror("Entro en el hijo del tercero");
                            // Redirect input
                            close(STDIN_FILENO);
                            dup(p2[0]);

                            // Close
                            close(p2[0]);
                            //close(p2[1]);

                            // Execution
                            getCompleteCommand(argvv, 2);
                            execvp(argv_execvp[0], argv_execvp);
                            perror("execvp");
                            exit(EXIT_FAILURE);
                        } else {
                            close(p2[0]);

                            if (!in_background) {
                                waitpid(pid_ls, &status, 0);
                                waitpid(pid_grep, &status, 0);
                                waitpid(pid_wc, &status, 0);
                            }
                        }
                    } else if (command_counter > 3) {
                        /*
                        pid_t a = fork();
                        if (a==0) {
                            getCompleteCommand(argvv, 0);
                            execvp(argv_execvp[0], argv_execvp);
                        } else {
                            waitpid(a, &status, 0);
                        }
                         */
                        // --------------- n commands -----------------------------------------------------------
                        int fd[command_counter - 1][2];
                        for (int i = 0; i < command_counter - 1; i++) {
                            if (pipe(fd[i]) == -1) {
                                perror("pipe");
                                exit(EXIT_FAILURE);
                            }
                        }
                        pid_t pid[command_counter];


                        for (int i = 0; i < command_counter; i++) {
                            pid[i] = fork();
                            if (pid[i] == -1) {
                                perror("fork\n");
                                exit(EXIT_FAILURE);
                            }
                            if (i == 0) {
                                if (pid[i] == 0) {
                                    // First process, redirect input if aplicable

                                    if (filev[0][0] != '0') {
                                        fd_open = open(filev[0], O_RDONLY);
                                        if (fd_open == -1) {
                                            perror("open");
                                            exit(EXIT_FAILURE);
                                        }
                                        close(STDIN_FILENO);
                                        dup(fd_open);
                                        close(fd_open);
                                    }

                                    // First process, redirect output
                                    close(STDOUT_FILENO);
                                    dup(fd[i][1]);

                                    // Close
                                    close(fd[i][1]);
                                    close(fd[i][0]);

                                    // Execution
                                    //perror("Primera ejecución\n");
                                    getCompleteCommand(argvv, i);
                                    execvp(argv_execvp[0], argv_execvp);
                                    perror("execvp");
                                    exit(EXIT_FAILURE);
                                } else {
                                    // Close
                                    close(fd[i][1]);
                                    wait(NULL);
                                }
                            }
                            if (i == command_counter - 1) {
                                // Last one
                                if (pid[i] == 0) {
                                    //printf("%d at line %d", filev[1][0], __LINE__);
                                    // Last process, redirect input
                                    close(STDIN_FILENO);
                                    if (strcmp(filev[1], "0") != 0) {
                                        fd_read = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                                        if (fd_read == -1) {
                                            perror("open");
                                            exit(EXIT_FAILURE);
                                        }
                                        if ((dupfd = dup(fd_read) == -1)) {
                                            perror("dup");
                                            exit(EXIT_FAILURE);
                                        }
                                        close(fd_read);
                                    } else {
                                        dup(fd[i - 1][0]);
                                        close(fd[i - 1][0]);
                                    }

                                    // // Close
                                    // close(fd[i-1][0]);
                                    // //close(fd[i-1][1]);

                                    // Execution
                                    //perror("Última ejecución\n");
                                    getCompleteCommand(argvv, i);
                                    execvp(argv_execvp[0], argv_execvp);
                                    perror("execvp");
                                    exit(EXIT_FAILURE);
                                } else {
                                    close(fd[i - 1][0]);
                                    if (!in_background) {
                                        for (int j = 0; j < num_commands; j++) {
                                            wait(NULL);
                                            waitpid(pid[j], &status, 0);
                                        }
                                    }
                                }
                            } else {
                                if (pid[i] == 0) {
                                    // Redirect input
                                    close(STDIN_FILENO);
                                    dup(fd[i - 1][0]);

                                    // Close
                                    close(fd[i - 1][0]);
                                    close(fd[i - 1][1]);

                                    // Redirect output
                                    close(STDOUT_FILENO);
                                    dup(fd[i][1]);

                                    // Close
                                    close(fd[i - 1][0]); // Input
                                    close(fd[i][1]);
                                    close(fd[i][0]);

                                    // Execution
                                    //perror("Ejecución número\n");
                                    getCompleteCommand(argvv, i);
                                    execvp(argv_execvp[0], argv_execvp);
                                    //perror("execvp");
                                    exit(EXIT_FAILURE);
                                } else {
                                    waitpid(pid[i], &status, 0);
                                    close(fd[i - 1][0]);
                                    close(fd[i][1]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}