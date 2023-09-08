#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

unsigned int countSetBits(unsigned int n);

int main(int argc, char *argv[]) {

    // Check if we attached file
    if (argc != 2) {
        printf("USAGE: ./biased_bits sample_file\n");
        exit(1);
    }

    char *filename = argv[1];  // Get filename
    FILE *fp = fopen(filename, "r");  // Open file pointer in read
    char *line = NULL;
    char *next_line = NULL;
    size_t len = 0;
    size_t next_len = 0;
    ssize_t read;

    if(fp == NULL) {
        perror(filename);
        exit(2);
    }

    int line_count = 0;

    // Check for invalids first
    while ((read = getline(&line, &len, fp)) != -1) {
        char *endptr;
        long num = strtol(line, &endptr, 2);
        int set_bits = countSetBits(num);
        size_t line_length = strlen(line)-1;
        // Check if it is not integer or too long
        if (*endptr != '\0' && *endptr != '\n' || line_length > 16) {
            printf("invalid: %s", line);
            fclose(fp);
            free(line);
            exit(0);
        }
        line_count++;
    }

    // Check if too short
    if (line_count < 2) {
        printf("not enough samples");
        fclose(fp);
        free(line);
        exit(0);
    }

    // Reopen file
    fclose(fp);
    fp = fopen(filename, "r");

    float lowest_percent = 100.0f;
    float highest_percent = 0.0f;

    // No errors so calculate percentage of set bits
    // (read = getline(&line, &len, fp)) != -1 && 
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

        // Find lowest and highest percentage
        if (div < lowest_percent) {
            lowest_percent = div;
            round(lowest_percent);
        }
        if (div > highest_percent) {
            highest_percent = div;
            round(highest_percent);
        }
        //printf("set_bits1 = %d\nset_bits2 = %d\nline_length1 = %d\nline_length2 = %d\npair_set_bits = %d\ntotal_length = %d\n\n", set_bits1, set_bits2, line_length1, line_length2, pair_set_bits, total_length);
        //printf("%d / %d = %.2f%%\n", pair_set_bits, total_length, div);
    }

    printf("%d%%\n%d%%\n", (int)lowest_percent, (int)highest_percent);

    // End program
    fclose(fp);
    free(line);
    free(next_line);
    exit(0);
}

/* 
Function to get number of set bits 
*/ 
unsigned int countSetBits(unsigned int n) {
    unsigned int count = 0;
    while (n) {
        count += n & 1;   // Bitwise AND with 1 to check least sig bit
        n >>= 1;  // Right shift to check next bit
    }
    return count;
}
