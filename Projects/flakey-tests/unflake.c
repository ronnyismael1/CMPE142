#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void handle_timeout(int s) {
    if (s == SIGALRM) {
        exit(100 + s);
    }
}
void print_usage() {
    printf("USAGE: ./unflake max_tries max_timeout test_command args...\n");
    printf("max_tries - must be greater than or equal to 1.\n");      
    printf("max_timeout - number of seconds greater than or equal to 1.\n");
    exit(1);
}
void close_unlink(int out, int err) { 
    close(out);
    close(err);
    unlink("stdout_tmp.txt");
    unlink("stderr_tmp.txt");
}

int main (int argc, char*argv[]) {
    if (argc < 4) {
        print_usage();
        exit(1);
    }
    int max_tries = atoi(argv[1]);
    int max_timeout = atoi(argv[2]);
    if (max_tries < 1 || max_timeout < 1) {
        print_usage();
        exit(1);
    }
    int stdout_fd = open("stdout_tmp.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    int stderr_fd = open("stderr_tmp.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    int status;
    char ppid_str[20];
    snprintf(ppid_str, sizeof(ppid_str), "%d", getpid()); // Convert main process PID to string
    setenv("UNFLAKE_PPID", ppid_str, 1); // Set it as an environment variable

    for (int i = 0; i < max_tries; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Failed to fork\n");
            exit(1);
        }
        if (pid == 0) {
            signal(SIGALRM, handle_timeout);
            alarm(max_timeout);
            dup2(stdout_fd, 1);
            dup2(stderr_fd, 2);
            close(stdout_fd);
            close(stderr_fd);
            execvp(argv[3], &argv[3]);
            perror(argv[3]);
            exit(2);
        } else {
            waitpid(pid, &status, 0);
            char buffer [1024] = {0};
            int bytes_read;
            lseek(stdout_fd, 0, SEEK_SET);
            while ((bytes_read = read(stdout_fd, buffer, sizeof(buffer))) > 0) {
                write (1, buffer, bytes_read);
            }
            lseek(stderr_fd, 0, SEEK_SET);
            while ((bytes_read = read(stderr_fd, buffer, sizeof(buffer))) > 0) {
                write (2, buffer, bytes_read);
            }   
            if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM) {
                close_unlink(stdout_fd, stderr_fd);
                exit(100 + SIGALRM);
            }
            if (WIFEXITED(status) && WEXITSTATUS(status) == 2) {
                close_unlink(stdout_fd, stderr_fd);
                exit(2);
            }
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                close_unlink(stdout_fd, stderr_fd);
                exit(0);
            }
        }
    }
    close_unlink(stdout_fd, stderr_fd);
    exit(WEXITSTATUS(status));
}
