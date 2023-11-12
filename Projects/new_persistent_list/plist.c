#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include "plist.h"

// Function to list the students in order
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
            printf("Found: %d %s\n", entry->student.id, entry->student.name);
            return;
        }
        offset = entry->next_offset;
    }
    printf("%d not found\n", id);
}

// Function to initialize an empty bag
void initialize_empty_bag(int fd) {
    // Create a 64K file filled with zeros
    lseek(fd, LIST_FILESIZE - 1, SEEK_SET);
    write(fd, "", 1);

    // Memory map file
    struct file_hdr *hdr = mmap(NULL, LIST_FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (hdr == MAP_FAILED) {
        perror("Error mapping file into memory");
        close(fd);
        exit(2);
    }

    // Initialize file header
    hdr->magic = FILE_MAGIC;
    hdr->first_student_off = 0;
    hdr->first_free_off = sizeof(struct file_hdr);

    // Initialize first free entry
    struct entry_s *first_free_entry = (struct entry_s *)offset2addr(hdr, hdr->first_free_off);
    first_free_entry->size = LIST_FILESIZE - sizeof(struct file_hdr);
    first_free_entry->magic = FREE_MAGIC;
    first_free_entry->next_offset = 0;

    // Unmap file
    munmap(hdr, LIST_FILESIZE);
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

int main(int argc, char **argv) {
    // Check for the -t flag and filename
    if (argc < 2) {
        printf("USAGE: ./plist [-t] filename\n");
        exit(1);
    }

    int t_flag = 0;
    char *filename = NULL;

    if (strcmp(argv[1], "-t") == 0) {
        t_flag = 1;
        if (argc < 3) {
            printf("USAGE: ./plist [-t] filename\n");
            exit(1);
        }
        filename = argv[2];
    } else {
        filename = argv[1];
    }

    // Open the file and memory map it
    int fd = open(filename, O_RDWR);
    if (fd == -1 && errno == ENOENT) {
        // File doesn't exist, create it
        fd = open(filename, O_RDWR | O_CREAT, 0666);
        initialize_empty_bag(fd);
    }

    struct stat st;

    // Add this for -t functionality
    int mmap_flag = t_flag ? MAP_PRIVATE : MAP_SHARED;
    void *file_in_memory = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, mmap_flag, fd, 0);

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, stdin)) != -1) {
        // Remove the newline character
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        // Conditions for functions
        if (strcmp(line, "l") == 0) {
            list_students((struct file_hdr *)file_in_memory);
        } else if (strncmp(line, "f ", 2) == 0) {
            int id = atoi(line + 2);
            find_student((struct file_hdr *)file_in_memory, id);
        } else if (strncmp(line, "a ", 2) == 0) {
            int id;
            char name[256];
            sscanf(line, "a %d %255[^\n]", &id, name);
            add_student((struct file_hdr *)file_in_memory, id, name);
        } else {
            printf("Invalid command %c. Possible commands are:\n", line[0]);
            printf("    l - list all the ids and students in the list\n");
            printf("    a id student_name - add a student with the given id and name\n");
            printf("    d id - delete the student with the given id\n");
            printf("    f id - find the student with the given id\n");
        }
        free(line);
        line = NULL;
    }

    // cleanup
    munmap(file_in_memory, st.st_size);
    close(fd);
    return 0;
}
