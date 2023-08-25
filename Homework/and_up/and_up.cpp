#include <stdio.h>
#include <stdlib.h>

#define MAX_WIDTH 16    // Max amounts of bits for visualizing

// Temporary function the help visualize binary bits
void printBinaryWithZeros(unsigned int number, int width) {
    for (int i = width - 1; i >= 0; i--) {
        printf("%d", (number >> i) & 1);
        if (i % 4 == 0) {
            printf(" ");
        }
    }
}

int main (int argc, char **argv) {
    // Checks if we provided any arguments
    if (argc < 2) {
        printf("USAGE: ./and_up numbers_to_and ...");
        exit (1);
    }
    unsigned int result;    // Unsigned to handle binary better
    char *p;                                          // Initialize result to be first arg
    result = strtol(argv[1], &p, 10);                 // ""
    printf("%u\t", result);                           // ""
    printBinaryWithZeros(result, MAX_WIDTH);          // ""
    printf("\n");                                     // ""

    // For loop to perform bitwise AND on arguments
    for (int i = 2; i < argc; i++) {
        char *p;
        long conv = strtol(argv[i], &p, 10);
        result &= conv;

        printf("%ld\t", conv);
        printBinaryWithZeros(conv, MAX_WIDTH);
        printf("\n");
    }

    // Prints results below
    printf("%ld\t", result);
    printBinaryWithZeros(result, MAX_WIDTH);
    printf("\n");
    exit(0);
}

// Temporary function to help convert decimal to binary
void printBinary(unsigned int number) {
    if (number > 1) {
        printBinary(number / 2);
    }
    printf("%d", number % 2);
};
