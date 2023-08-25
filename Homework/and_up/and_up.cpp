#include <stdio.h>
#include <stdlib.h>

#define MAX_WIDTH 16    // Max amounts of bits for visualizing

// Temporary function the help visualize binary bits
void printBinary(unsigned int number, int width);

int main (int argc, char **argv) {
    // Checks if we provided any arguments
    if (argc < 2) {
        printf("USAGE: ./and_up numbers_to_and ...");
        exit (1);
    }

    unsigned int result;    // Unsigned to handle binary better
    char *p;                                          
    result = 0xFFFF;        // Initialize result
    // For loop to perform bitwise AND on arguments
    for (int i = 1; i < argc; i++) {
        char *p;
        long conv = strtol(argv[i], &p, 0); // Base 0 to auto-detect if it is hex, dec, or octal
        result &= conv;
        
        printf("%ld\t", conv);
        printBinary(conv, MAX_WIDTH);
        printf("\n");
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