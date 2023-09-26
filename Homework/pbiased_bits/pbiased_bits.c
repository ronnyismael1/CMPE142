#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

unsigned int countSetBits(unsigned int n) {
    unsigned int count = 0;
    while (n) {
        count += n & 1;   // Bitwise AND with 1 to check least sig bit
        n >>= 1;  // Right shift to check next bit
    }
    return count;
}
void updatePercentage(float *lowest, float *highest, float current) {
    if (current < *lowest) {
        *lowest = current;
        round(*lowest);
    }
    if (current > *highest) {
        *highest = current;
        round(*highest);
    }
}
void clean(FILE *fileptr, char *lp1, char *lp2) {
    fclose(fileptr);
    free(lp1);
    free(lp2);
}
void processFile(char *filename, float *lowest, float *highest) {
    FILE *fp = fopen(filename, "r");  // Open file pointer in read
    char *line = NULL;
    char *next_line = NULL;
    size_t len = 0;
    size_t next_len = 0;
    ssize_t read;
    if(fp == NULL) {
        perror("hello");
        exit(2);
    }
    int line_count = 0;
    // Check for invalids first
    while ((read = getline(&line, &len, fp)) != -1) {
        char *endptr;
        long num = strtol(line, &endptr, 2);
        int set_bits = countSetBits(num);
        size_t line_length = strlen(line)-1;
        // Check if it is not integer, too long, or line is blank
        if ( *endptr != '\0' && *endptr != '\n' || line_length > 16 || *line == '\n') {
            printf("invalid: %s %s", line, filename);
            clean(fp, line, next_line);
            exit(2);
        }
        line_count++;
    }

    // Check if too short
    if (line_count < 2) {
        printf("not enough samples @%s\n", filename);
        clean(fp, line, next_line);
        exit(2);
    }

    // Reopen file
    fclose(fp);
    fp = fopen(filename, "r");

    float lowest_percent = 100.0f;
    float highest_percent = 0.0f;

    // No errors so calculate percentage of set bits
    while ((read = getline(&line, &len, fp)) != -1 && getline(&next_line, &next_len, fp) != -1) { 
        char *endptr, *next_endptr;

        long num1 = strtol(line, &endptr, 2);
        long num2 = strtol(next_line, &next_endptr, 2);

        int set_bits1 = countSetBits(num1);   // Count set bits in num1
        int set_bits2 = countSetBits(num2);   // Count set bits in num2

        size_t line_length1 = strlen(line)-1;
        size_t line_length2 = strlen(next_line)-1;

        int pair_set_bits = set_bits1 + set_bits2;
        int total_length = line_length1 + line_length2;

        float div = (pair_set_bits*1.0f / total_length*1.0f)*100.0f;
        updatePercentage(&lowest_percent, &highest_percent, div);
    }

    // printf("%d%%\n%d%%\n", (int)lowest_percent, (int)highest_percent);

    // End program
    clean(fp, line, next_line);
    *lowest = lowest_percent;
    *highest = highest_percent;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("USAGE: ./pbiased_bits sample_file ...\n");
        exit(1);
    }
    int pipefd[2];
    float overall_min = 100.0f;
    float overall_max = 0.0f;
    for (int i = 1; i < argc; i++) {
        pipe(pipefd); // Create a new pipe for each file
        pid_t pid = fork();
        if (pid == 0) { // Child process
            close(pipefd[0]);
            float low, high;
            processFile(argv[i], &low, &high);
            write(pipefd[1], &low, sizeof(float));
            write(pipefd[1], &high, sizeof(float));
            close(pipefd[1]);
            exit(0);
        } else {
            close(pipefd[1]);
        }
        int status;
        wait(&status);  // Get the exit status of the child process

        if (WIFEXITED(status) && WEXITSTATUS(status) == 2) {
            // Invalid found, so break out of the loop
            exit(2);
        } else if (WIFEXITED(status) && WEXITSTATUS(status) != 2) {
            float results[2];
            read(pipefd[0], results, sizeof(float) * 2);
            if (results[0] < overall_min) overall_min = results[0];
            if (results[1] > overall_max) overall_max = results[1];
        }
        close(pipefd[0]);

    }
    char buffer[100];
    int length = sprintf(buffer, "%d%%\n%d%%\n", (int)overall_min, (int)overall_max);
    write(STDOUT_FILENO, buffer, length);
    exit(0);
}
