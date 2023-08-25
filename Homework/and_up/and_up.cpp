#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv) {
    if (argc < 2) {
        printf("USAGE: ./and_up numbers_to_and ...");
        exit (1);
    }
    for (int i = 1; i < argc; i++) {
        char *p;
        long conv = strtol(argv[i], &p, 10);

        printf("%ld\n", conv);
    }
    exit(0);
}
