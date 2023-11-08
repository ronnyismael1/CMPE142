#include <stdio.h>
#include <stdlib.h>

void print_words_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");  // Open the file for reading
    if (file == NULL) {
        perror(filename);
        return;
    }

    char *word = NULL;
    while (fscanf(file, "%ms", &word) == 1) {
        printf("%s\n", word);
        free(word);  // Free the memory allocated by fscanf
        word = NULL; // Set word to NULL to be safe for the next iteration
    }

    fclose(file); // Close the file
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file1> [file2 ...]\n", argv[0]);
        return 1;
    }

    // Assuming we only want to test the first file for now
    print_words_from_file(argv[1]);

    return 0;
}
