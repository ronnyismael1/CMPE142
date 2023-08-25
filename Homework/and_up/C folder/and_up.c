#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv) {
    // Checks if we provided any arguments
    if (argc < 2) {
        printf("USAGE: ./and_up numbers_to_and ...\n");
        exit (1);
    }
    unsigned int result = 0xFFFF;   // Unsigned to handle binary better
    char *p;                                          
    // For loop to perform bitwise AND on arguments
    for (int i = 1; i < argc; i++) {
        long conv = strtol(argv[i], &p, 0); // Base 0 to auto-detect if it is hex, dec, or octal
        // If it is not a number
        if (*p) { // If p is pointing to non-null then it means argv[i] wasn't fully a number
            printf("%s is not a number\n", argv[i]);
            for (int j = 0; argv[i][j] != '\0'; j++) {
                conv = argv[i][j];
                result &= conv;
            }
        // If it is a decimal, hex, or octal
        }else {
            result &= conv;
        }
    }
    // Prints results below
    printf("%ld", result);
    exit(0);
}
