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

    // Flags to help with string output
    int isString = 0;
    int firstWordPrinted = 0;

    // Check if string exists
    for (int i = 1; i < argc; i++) {
        long conv = strtol(argv[i], &p, 0); // Base 0 to auto-detect if it is hex, dec, or octal
        if (*p) { // If p is pointing to non-null then it means argv[i] wasn't fully a number
            isString = 1;
            break;
        }
    }

    // Print first word and BITWISE ANDing
    for (int i = 1; i < argc; i++) {

        long conv = strtol(argv[i], &p, 0);
        
        // If-condition to print first word only once
        if (*p && !firstWordPrinted) {
            char firstWord[100];
            int j = 0;

            // Copy characters until we find a space or the end of the string
            while (argv[i][j] != ' ' && argv[i][j] != '\0') {
                firstWord[j] = argv[i][j];
                j++;
            }
            firstWord[j] = '\0';    // null-terminate the first word
            printf("%s is not a number\n", firstWord);
            firstWordPrinted = 1;
        }

        // Bitwise ANDing
        if (isString || *p) {   // If it is not a number or string flag is set
            for (int j = 0; argv[i][j] != '\0'; j++) {
                conv = argv[i][j];
                result &= conv;
            }
        }else { // If it is a decimal, hex, or octal
            result &= conv;
        }
    }
    // Prints results below
    printf("%ld\n", result);
    exit(0);
}
