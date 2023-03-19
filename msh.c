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
                    exit(0);
                } else if (strcmp(argvv[0][0], "mytime") == 0) {
                    printf("mytiming...\n");
                    printf("mytime: %lu\n", mytime);
                } else if (strcmp(argvv[0][0], "cd") == 0) {
                    printf("changing directory...\n");
                    if (argvv[0][1] == NULL) {
                        chdir(getenv("HOME"));
                    } else {
                        chdir(argvv[0][1]);
                    }

                // mycalc
                } else if (strcmp(argvv[0][0], "mycalc") == 0) {
                    if ((argvv[0][1] == NULL) || (argvv[0][2] == NULL) || (argvv[0][3] == NULL) || (argvv[0][4] != NULL)) {
                        printf("[ERROR] The structure of the \033[1mcommand\033[0m is mycalc <operand_1> <add/mul/div> <operand_2>\n");
                    } else {
                        //printf("%s yes", argvv[0][0]);
                        //printf("%s no", argvv[0][1]);
                        //printf("calculating\n");
                        //print_command(argvv, filev, in_background);
                        // 1. Get the first number
                        int first_number = atoi(argvv[0][1]);
                        // 2. Get the second number
                        int second_number = atoi(argvv[0][3]);
                        //printf("result: %i", first_number+second_number);
                        // 3. Get the operator + result
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
                            printf("[ERROR] The structure of the \033[1mcommand\033[0m is mycalc <operand_1> <add/mul/div> <operand_2>\n");
                        }
                    }
                /*
                    //printf("%s yes", argvv[0][0]);
                    //printf("%s no", argvv[0][1]);
                    //printf("calculating\n");
                    //print_command(argvv, filev, in_background);
                    // 1. Get the first number
                    int first_number = atoi(argvv[0][1]);
                    // 2. Get the second number
                    int second_number = atoi(argvv[0][3]);
                    //printf("result: %i", first_number+second_number);
                    // 3. Get the operator + result
                    char operator_str[3];
                    strcpy(operator_str, argvv[0][2]);
                    if (strcmp(operator_str, "add") == 0) {
                        // 4a. If the operator is sum, store it in Acc
                        int result = first_number + second_number;
                        Acc += result;
                        printf("[OK] %i + %i = %i; Acc %i", first_number, second_number, result, Acc);
                    } else if (operator_str == "mul") {
                        int result = first_number * second_number;
                        printf("[OK] %i * %i = %i", first_number, second_number, result);
                    } else if (operator_str == "div") {
                        int result = first_number / second_number;
                        float reminder = first_number % second_number;
                        printf("[OK] %i / %i = %i, Reminder %f", first_number, second_number, result, reminder);


                        printf("Invalid operator");
                    }
                */








                } else {

                    // Execute commands in a loop
                    //
                    // 1. Create a child process
                    // 2. Child process: execute the command
                    // 3. Parent process: wait for the child to finish
                    // 4. If in_background is 0, parent waits. Otherwise, parent continues
                    // 5. Shell shows a new prompt
                    // 6. Shell loops back to step 1

                    // 1. Create a child process:
                    pid_t pid = fork();

                    // 2. Child process: execute the command:
                    if (pid == 0) {
                        execvp(argvv[0][0], argvv[0]);
                    } else if (pid > 0) {
                        // 3. Parent process: wait for the child to finish:
                        if (in_background == 0)
                            waitpid(pid, &status, 0);
                    }
                }
            }
        }
    }
    return 0;
}
