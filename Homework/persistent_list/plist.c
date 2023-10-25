#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/stat.h>
#include "plist.h"

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
    // If we reach here, the student was not found
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

// Function to add a student
void add_student(struct file_hdr *hdr, int id, const char *name) {
    uint32_t offset = hdr->first_student_off;
    uint32_t prev_offset = 0;
    // Check if the student with the ID already exists and find the right position
    while (offset != 0) {
        struct entry_s *entry = (struct entry_s *)offset2addr(hdr, offset);
        if (entry->magic == STUDENT_MAGIC) {
            if (entry->student.id == id) {
                printf("%d already present for %s\n", id, entry->student.name);
                return;
            } else if (entry->student.id > id) {
                // Found the position where the new student should be inserted
                break;
            }
        }
        prev_offset = offset;
        offset = entry->next_offset;
    }
    // Size needed for the new student
    uint16_t required_size = sizeof(struct entry_s) + strlen(name) + 1;  // +1 for null terminator
    // Find a suitable free space
    uint32_t free_offset = hdr->first_free_off;
    uint32_t prev_free_offset = 0;
    struct entry_s *prev_free_entry = NULL;
    while (free_offset != 0) {
        struct entry_s *free_entry = (struct entry_s *)offset2addr(hdr, free_offset);
        if (free_entry->size >= required_size) {
            int remaining_space = free_entry->size - required_size;
            if (remaining_space < sizeof(struct entry_s)) {
                required_size = free_entry->size;
                remaining_space = 0;
            }
            struct entry_s *new_student = (struct entry_s *)((char *)free_entry + remaining_space);
            new_student->size = required_size;
            new_student->magic = STUDENT_MAGIC;
            new_student->next_offset = offset;  // The offset where the new student should point to next
            new_student->student.id = id;
            strcpy(new_student->student.name, name);
            if (remaining_space == 0) {
                if (prev_free_entry) {
                    prev_free_entry->next_offset = free_entry->next_offset;
                } else {
                    hdr->first_free_off = free_entry->next_offset;
                }
            } else {
                free_entry->size = remaining_space;
            }
            if (prev_offset) {
                struct entry_s *prev_entry = (struct entry_s *)offset2addr(hdr, prev_offset);
                prev_entry->next_offset = free_offset + remaining_space;  // Point the previous student to the new student
            } else {
                hdr->first_student_off = free_offset + remaining_space;  // The new student is the first in the list
            }
            printf("%d %s added\n", id, name);
            return;
        }
        prev_free_offset = free_offset;
        prev_free_entry = free_entry;
        free_offset = free_entry->next_offset;
    }
    // If we reach here, there's no space left for the new student
    printf("out of space\n");
}

int main(int argc, char *argv[]) {
    // Check command-line arguments
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
    int flags = O_RDWR;
    int fd = open(filename, flags);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Memory map the file
    int mmap_flags = test_mode ? MAP_PRIVATE : MAP_SHARED;
    struct file_hdr *hdr = mmap(NULL, LIST_FILESIZE, PROT_READ | PROT_WRITE, mmap_flags, fd, 0);
    if (hdr == MAP_FAILED) {
        perror("Error mmapping the file");
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
            } else {
                printf("Invalid input format for 'f' command\n");
            }
        } else if (line[0] == 'a' && line[1] == ' ') {
            int id;
            char name[256];  // Assuming max name length of 256 for simplicity
            if (sscanf(line + 2, "%d %255[^\n]", &id, name) == 2) {
                add_student(hdr, id, name);
            } else {
                printf("Invalid input format for 'a' command\n");
            }
        }
        free(line);
        line = NULL;
    }
    munmap(hdr, LIST_FILESIZE);
    close(fd);
    exit(0);
}
