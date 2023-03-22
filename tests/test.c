#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file extension> <pattern>\n", argv[0]);
        return 1;
    }

    char* file_extension = argv[1];
    char* pattern = argv[2];
    int fd1[2], fd2[2];

    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        fprintf(stderr, "Pipe failed");
        return 1;
    }

    pid_t pid_ls = fork();

    if (pid_ls < 0) {
        fprintf(stderr, "Fork failed");
        return 1;
    }

    if (pid_ls == 0) {
        // Child process 1 (ls)
        close(fd1[0]); // Close unused read end of pipe 1
        dup2(fd1[1], STDOUT_FILENO); // Redirect stdout to pipe 1 write end
        execlp("ls", "ls", NULL); // Execute ls command
        fprintf(stderr, "Failed to execute ls command");
        exit(1);
    }

    pid_t pid_grep1 = fork();

    if (pid_grep1 < 0) {
        fprintf(stderr, "Fork failed");
        return 1;
    }

    if (pid_grep1 == 0) {
        // Child process 2 (grep1)
        close(fd1[1]); // Close write end of pipe 1
        dup2(fd1[0], STDIN_FILENO); // Redirect stdin to pipe 1 read end
        close(fd2[0]); // Close read end of pipe 2
        dup2(fd2[1], STDOUT_FILENO); // Redirect stdout to pipe 2 write end
        execlp("grep", "grep", file_extension, NULL); // Execute grep1 command
        fprintf(stderr, "Failed to execute grep1 command");
        exit(1);
    }

    pid_t pid_grep2 = fork();

    if (pid_grep2 < 0) {
        fprintf(stderr, "Fork failed");
        return 1;
    }

    if (pid_grep2 == 0) {
        // Child process 3 (grep2)
        close(fd2[1]); // Close write end of pipe 2
        dup2(fd2[0], STDIN_FILENO); // Redirect stdin to pipe 2 read end
        execlp("grep", "grep", pattern, NULL); // Execute grep2 command
        fprintf(stderr, "Failed to execute grep2 command");
        exit(1);
    }

    close(fd1[0]); // Close both ends of pipe 1 in parent process
    close(fd1[1]);
    close(fd2[0]); // Close both ends of pipe 2 in parent process
    close(fd2[1]);

    // Wait for child processes to finish
    waitpid(pid_ls, NULL, 0);
    waitpid(pid_grep1, NULL, 0);
    waitpid(pid_grep2, NULL, 0);

    return 0;
}
