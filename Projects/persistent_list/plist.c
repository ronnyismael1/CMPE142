#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/stat.h>
#include "plist.h"

#define addr2offset(hdr, addr) ((uint32_t)((char *)(addr) - (char *)(hdr)))

// gcc -g -std=gnu2x -Wall -o plist plist.c
// /mnt/c/users/rismael/documents/sjsu/F23/cmpe142/homework/persistent_list

// Function to list all students
void list_students(struct file_hdr *hdr) {
    uint32_t offset = hdr->first_student_off;
    if (offset == 0) {
        printf("empty list\n");
        return;
    }
    while (offset != 0) {
        struct entry_s *entry = (struct entry_s *)offset2addr(hdr, offset);
        if (entry->magic == STUDENT_MAGIC) {
            printf("%d %s\n", entry->student.id, entry->student.name);
        }
        offset = entry->next_offset;
    }
}

// Function to find a student by ID
void find_student(struct file_hdr *hdr, int id) {
    uint32_t offset = hdr->first_student_off;
    while (offset != 0) {
        struct entry_s *entry = (struct entry_s *)offset2addr(hdr, offset);
        if (entry->magic == STUDENT_MAGIC && entry->student.id == id) {
            printf("found: %d %s\n", entry->student.id, entry->student.name);
            return;
        }
        offset = entry->next_offset;
    }
    printf("%d not found\n", id);
}

// Function to initialize a new file
void initialize_file(struct file_hdr *hdr) {
    // Set the magic number and initial offsets
    hdr->magic = FILE_MAGIC;
    hdr->first_student_off = 0;

    // Calculate the offset for the first free entry
    uint32_t free_entry_offset = sizeof(struct file_hdr);
    hdr->first_free_off = free_entry_offset;

    // Create the first free entry right after the header
    struct entry_s *first_free_entry = (struct entry_s *)offset2addr(hdr, free_entry_offset);
    first_free_entry->size = LIST_FILESIZE - sizeof(struct file_hdr) - sizeof(struct entry_s);
    first_free_entry->magic = FREE_MAGIC;
    first_free_entry->next_offset = 0;  // No subsequent entries yet
}

// Function to add a student by ID and name
void add_student(struct file_hdr *hdr, int id, char *name) {
    uint32_t offset = hdr->first_student_off;
    struct entry_s *prev_student_entry = NULL;
    // Find the correct position
    while (offset != 0) {
        struct entry_s *entry = (struct entry_s *)offset2addr(hdr, offset);
        if (entry->magic == STUDENT_MAGIC) {
            if (entry->student.id == id) {
                printf("%d already present for %s\n", id, entry->student.name);
                return;
            } else if (entry->student.id > id) {
                break;
            }
            prev_student_entry = entry;
        }
        offset = entry->next_offset;
    }
    // Find a free space
    uint32_t prev_offset = 0;
    offset = hdr->first_free_off;
    while (offset != 0) {
        struct entry_s *entry = (struct entry_s *)offset2addr(hdr, offset);
        size_t required_size = sizeof(struct entry_s) + strlen(name) + 1;
        if (entry->magic == FREE_MAGIC && entry->size >= required_size) {
            // Split the free space
            if (entry->size > required_size + sizeof(struct entry_s)) {
                uint32_t new_offset = offset + required_size;
                struct entry_s *new_entry = (struct entry_s *)offset2addr(hdr, new_offset);
                new_entry->size = entry->size - required_size;
                new_entry->magic = FREE_MAGIC;
                new_entry->next_offset = entry->next_offset;
                entry->size = required_size;
            }
            // Adding the student
            entry->magic = STUDENT_MAGIC;
            entry->student.id = id;
            strcpy(entry->student.name, name);
            if (prev_student_entry) {
                entry->next_offset = prev_student_entry->next_offset;
                prev_student_entry->next_offset = offset;
            } else {
                entry->next_offset = hdr->first_student_off;
                hdr->first_student_off = offset;
            }
            printf("%d %s added\n", id, name);
            // Update the first_free_off in the header
            if (hdr->first_free_off == offset) {
                hdr->first_free_off = offset + required_size;
            }
            return;
        }
        prev_offset = offset;
        offset = entry->next_offset;
    }
    // Reaching here no insertion
    printf("out of space\n");
}

int main(int argc, char *argv[]) {
    // Check command line arguments
    if (argc < 2 || argc > 3 || (argc == 3 && strcmp(argv[1], "-t") != 0)) {
        printf("USAGE: ./plist [-t] filename\n");
        exit(1);
    }
    bool test_mode = false;
    char *filename;
    if (argc == 3) { 
        test_mode = true;
        filename = argv[2];
    } else {
        filename = argv[1];
    }

    // Open the file
    int flags = O_RDWR | O_CREAT;
    int fd = open(filename, flags, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    struct stat st;
    fstat(fd, &st);
    if (st.st_size == 0) { // It's a new file
        if (ftruncate(fd, LIST_FILESIZE) == -1) {
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    // Memory map the file
    int mmap_flags = test_mode ? MAP_PRIVATE : MAP_SHARED;
    struct file_hdr *hdr = mmap(NULL, LIST_FILESIZE, PROT_READ | PROT_WRITE, mmap_flags, fd, 0);
    if (hdr == MAP_FAILED) {
        exit(EXIT_FAILURE);
    }

    // Check if file needs to be initialized
    if (hdr->magic != FILE_MAGIC) {
        initialize_file(hdr);
    }

    // Command handling loop
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, stdin) != -1) {
        // Handle newline character
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        // Handle commands
        if (line[0] == 'l' && strlen(line) == 1) {
            list_students(hdr);
        } else if (line[0] == 'f' && line[1] == ' ') {
            int id;
            if (sscanf(line + 2, "%d", &id) == 1) {
                find_student(hdr, id);
            }
        } else if (line[0] == 'a' && line[1] == ' ') {
            int id;
            char name[256];
            if (sscanf(line + 2, "%d %255[^\n]", &id, name) == 2) {
                add_student(hdr, id, name);
            }
        }
        else {
            printf("Invalid command %c. Possible commands are:\n", line[0]);
            printf("    l - list all the ids and students in the list\n");
            printf("    a id student_name - add a student with the given id and name\n");
            printf("    d id - delete the student with the given id\n");
            printf("    f id - find the student with the given id\n");
        }
        free(line);
        line = NULL;
    }
    munmap(hdr, LIST_FILESIZE);
    close(fd);
    exit(0);
}
