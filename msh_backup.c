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
                    while (1){
                        exit(0);
                    }
                    //exit(0);
                } else if (strcmp(argvv[0][0], "mytime") == 0) {
                    //printf("mytiming...\n");
                    /*
                    float reduced_time = mytime / 1000; // Seconds
                    reduced_time = reduced_time / 60; // Minutes
                    //reduced_time = reduced_time / 60; // Hours
                    int hours, minutes, seconds = 0;
                    hours = reduced_time / 3600;
                    reduced_time = reduced_time - (hours * 3600);
                    minutes = reduced_time / 60;
                    reduced_time = reduced_time - (minutes * 60);
                    seconds = reduced_time;
                    printf("mytime: %i:%i:%i\n", hours, minutes, seconds);
                     */
                    // Expressing the time from miliseconds to format HH:MM:SS
                    int hours = mytime / 3600000;
                    int minutes = (mytime % 3600000) / 60000;
                    int seconds = ((mytime % 3600000) % 60000) / 1000;
                    printf("mytime: %02d:%02d:%02d\n", hours, minutes, seconds);

                    //printf("mytime: %lu\n", mytime);
                } else if (strcmp(argvv[0][0], "cd") == 0) {
                    //printf("changing directory...\n");
                    if (argvv[0][1] == NULL) {
                        chdir(getenv("HOME"));
                    } else {
                        chdir(argvv[0][1]);
                    }

                // mycalc
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
                } else {

                    // Calls to function for each command
                    // Only 1 command works here, no pipes
                    pid_t pid = fork();

                    // 2. Child process: execute the command:
                    if (pid == 0) {
                        //char *argv[] = {"nohub sleep", "3", "&", NULL};
                        //execvp(argv[0], argv);
                        execvp(argvv[0][0], argvv[0]);

                    } else if (pid > 0) {
                        // 3. Parent process: wait for the child to finish:
                        if (in_background == 0)
                            waitpid(pid, &status, 0);
                    }

                    /*
                    // 3 Commands
                    int pipefd[3][2]; // Pipes
                    for (int i = 0; i < num_commands-1; i++) {
                        if (pipe(pipefd[i]) == -1) {
                            perror("pipe");
                            exit(EXIT_FAILURE);
                        }
                    }

                    pid_t pid1 = fork();
                    if (pid1 == -1){
                        perror("fork1");
                        exit(EXIT_FAILURE);
                    } else if (pid1 == 0){
                        close(pipefd[0][0]); // Close unused read end of first pipe ----------------------------------
                        dup2(pipefd[0][1], STDOUT_FILENO); // redirect standard output to write end of first pipe
                        close(pipefd[1][0]); // close unused ends of second and third pipes
                        close(pipefd[1][1]);
                        close(pipefd[2][0]);
                        close(pipefd[2][1]);
                        getCompleteCommand(argvv, 0);
                        execvp(argv_execvp[0], argv_execvp);
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    }

                    pid_t pid2 = fork();
                    if (pid2 == -1){
                        perror("fork2");
                        exit(EXIT_FAILURE);
                    } else if (pid2 == 0){
                        close(pipefd[0][1]);
                        dup2(pipefd[0][0], STDIN_FILENO);
                        close(pipefd[1][0]);
                        dup2(pipefd[1][1], STDOUT_FILENO);
                        close(pipefd[2][0]);
                        close(pipefd[2][1]);
                        getCompleteCommand(argvv, 1);
                        execvp(argv_execvp[0], argv_execvp);
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    }

                    pid_t pid3 = fork();
                    if (pid3 == -1){
                        perror("fork3");
                        exit(EXIT_FAILURE);
                    } else if (pid3 == 0){
                        close(pipefd[0][0]);
                        close(pipefd[0][1]);
                        close(pipefd[1][1]);
                        dup2(pipefd[1][0], STDIN_FILENO);
                        close(pipefd[2][0]);
                        dup2(pipefd[2][1], STDOUT_FILENO);
                        getCompleteCommand(argvv, 2);
                        execvp(argv_execvp[0], argv_execvp);
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    }

                    close(pipefd[0][0]);
                    close(pipefd[0][1]);
                    close(pipefd[1][0]);
                    close(pipefd[1][1]);
                    close(pipefd[2][1]);



                    // Read from pipe and print to stdout, unless redirect exists
                    char buf[1024];
                    int n;
                    while ((n = read((long int)pipefd[2], buf, sizeof(buf))) > 0) {
                        write(STDOUT_FILENO, buf, n); // Write to standard output
                    }
                        close(pipefd[2][0]);

                    wait(NULL); // Wait for all children to finish
                    return 0;
                     */
                    /*
                    // Wait for all children to finish
                    if (in_background == 1) {
                        waitpid(pid1, &status, 0);
                        waitpid(pid2, &status, 0);
                        waitpid(pid3, &status, 0);
                    */

                }
            }
        }
    }
    return 0;
}
