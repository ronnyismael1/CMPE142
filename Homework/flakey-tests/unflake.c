#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// cd /mnt/c/users/rismael/documents/sjsu/f23/cmpe142/homework/flakey-tests
// gcc -g -std=gnu2x -Wall -o unflake unflake.c

void handle_timeout(int s) {    // If test command takes too long
    if (s == SIGALRM) {
        exit(100 + s);
    }
}

void print_usage(){
    printf("USAGE: ./unflake max_tries max_timeout test_command args...\n");
    printf("max_tries - must be greater than or equal to 1\n");      
    printf("max_timeout - number of seconds greater than or equal to 1\n");
    exit(1);
}

int main (int argc, char*argv[]) {
    // If not enough arguments
    if (argc < 4) {
        print_usage();
        exit(1);
    }
    // If max_tries or max_timeout is not a valid number and greather than or equal to 1
    int max_tries = atoi(argv[1]);
    int max_timeout = atoi(argv[2]);
    if (max_tries < 1 || max_timeout < 1) {
        print_usage();
        exit(1);
    }

    // Create temporary files for stdout and stderr
    int stdout_fd = open("stdout_tmp.txt, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR");
    int stderr_fd = open("stderr_tmp.txt, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR");

    pid_t pid = fork();     // Create child process
    if (pid < 0) {
        perror("Failed to fork\n");
        exit(1);
    }

    if (pid == 0) {     // This block runs in child process
        signal(SIGALRM, handle_timeout);    // Timeout signal handler
        alarm(max_timeout); // Alarm for max_timeout seconds

        dup2(stdout_fd, 1); // Redirect stdout to this file
        dup2(stderr_fd, 2); // Redirect stderr to this file

        close(stdout_fd); // Close original file
        close(stderr_fd); // Close original file

        execvp(argv[3], &argv[3]);  // Execute the test command, lines under will not execute
        perror(argv[3]);    // Will only run if execvp fails
        exit(2);
    } else {    // This block runs in parent process
        int status;
        waitpid(pid, &status, 0);   // Wait for child process to finish

        // TODO: Handle the exit status and other conditions

        // Output the contents of temp files to stdout & stderr
        char buffer [1024];
        int bytes_read;
        // Print stdout
        lseek(stdout_fd, 0, SEEK_SET);
        while ((bytes_read = read(stdout_fd, buffer, sizeof(buffer))) > 0) {
            write (1, buffer, bytes_read);
        }
        // Print stderr
        lseek(stderr_fd, 0, SEEK_SET);
        while ((bytes_read = read(stderr_fd, buffer, sizeof(buffer))) > 0) {
            write (2, buffer, bytes_read);
        }
        // Clean temp files
        close(stdout_fd); // Close original file
        close(stderr_fd); // Close original file
        unlink("stdout_tmp.txt");
        unlink("stderr_tmp.txt");
    }

    exit(0);
}
