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
    char *line = NULL;  // For getline
    size_t len = 0;     // ""
    ssize_t read;       // ""

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

        if (*endptr != '\0' && *endptr != '\n') {
            printf("invalid: %s", line);
            fclose(fp);
            free(line);
            exit(0);
        }
        if (line_length > 16) {
            printf("invalid: %s", line);
            fclose(fp);
            free(line);
            exit(0);        
        }
        line_count++;
    }

    if (line_count < 2) {
        printf("not enough samples");
        fclose(fp);
        free(line);
        exit(0);
    }

    // Reopen file
    fclose(fp);
    fp = fopen(filename, "r");

    int lowest_percent = 0;
    int highest_percent = 100;

    // Read to dat file
    while ((read = getline(&line, &len, fp)) != -1) { 
        char *endptr;
        long num = strtol(line, &endptr, 2);
        int set_bits = countSetBits(num);   // Count bits in num

        printf("Line: %sSet bits: %d\n", line, set_bits); // Print the line and set bit count
    }
    fclose(fp);
    free(line);
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
