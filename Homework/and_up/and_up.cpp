#include <stdio.h>
#include <stdlib.h>
#include <cctype>

#define MAX_WIDTH 16    // Max amounts of bits for visualizing

// Temporary function the help visualize binary bits
void printBinary(unsigned int number, int width);

int main (int argc, char **argv) {
    // Checks if we provided any arguments
    if (argc < 2) {
        printf("USAGE: ./and_up numbers_to_and ...");
        exit (1);
    }
    unsigned int result = 0xFFFF;   // Unsigned to handle binary better
    char *p;                                          
    // For loop to perform bitwise AND on arguments
    for (int i = 1; i < argc; i++) {
        long conv = strtol(argv[i], &p, 0); // Base 0 to auto-detect if it is hex, dec, or octal
        if (*p) { // If p is pointing to non-null then it means argv[i] wasn't fully a number
            //printf("%s is not a number\n", argv[i]);
            unsigned int temp = 0xFFFF;
            for (int j = 0; argv[i][j] != '\0'; j++) {
                conv = argv[i][j];
                result &= conv;
                printf("%c\t", argv[i][j]);
                printBinary(conv, MAX_WIDTH);
                printf("\n");
            }
        }else {
            result &= conv;
            printf("%ld\t", conv);
            printBinary(conv, MAX_WIDTH);
            printf("\n");
        }
    }
    // Prints results below
    printf("----------------------------\n%ld\t", result);
    printBinary(result, MAX_WIDTH);
    printf("\n");
    exit(0);
}

// Temporary function
void printBinary(unsigned int number, int width) {
    for (int i = width - 1; i >= 0; i--) {
        printf("%d", (number >> i) & 1);
        if (i % 4 == 0) {
            printf(" ");
        }
    }
}