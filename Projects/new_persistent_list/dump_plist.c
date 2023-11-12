#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/stat.h>
#include "plist.h"

struct list_node {
    struct list_node *next;
    struct entry_s *entry;
    uint32_t offset;
};

void add_to_list(struct list_node **head, struct entry_s *entry, uint32_t offset) {
    struct list_node *new_node = malloc(sizeof *new_node);
    new_node->entry = entry;
    new_node->offset = offset;
    new_node->next = NULL;
    // we call it head, but head points to the previous link in the chain
    // where we want to insert
    while (*head && (*head)->entry->student.id < entry->student.id) head = &(*head)->next;
    new_node->next = *head;
    *head = new_node;
}

int main(int argc, char **argv) {
    printf("%d\n", sizeof(struct entry_s));
    if (argc != 3 || (strcmp(argv[1], "dump") != 0 && strcmp(argv[1], "check") != 0)) {
        printf("USAGE: %s check|dump plist_file\n", argv[0]);
        exit(1);
    }
    bool dump = strcmp(argv[1], "dump") == 0;
    int fd = open(argv[2], O_RDONLY);
    if (fd == -1) {
        perror(argv[1]);
        exit(2);
    }

    struct stat stat;
    if (fstat(fd, &stat) == -1) {
        perror(argv[2]);
        exit(2);
    }
    
    if (stat.st_size != LIST_FILESIZE) {
        fprintf(stderr, "BAD FILE SIZE %ld should be %ld\n", stat.st_size, LIST_FILESIZE);
    }
    struct file_hdr *hdr = (struct file_hdr *)mmap(NULL, LIST_FILESIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (hdr == MAP_FAILED) {
        perror(argv[2]);
        exit(2);
    }

    bool error_found = false;

    if (dump) {
        printf("Magic %.4s\n", (char*)&hdr->magic);
        printf("Free offset %d\n", hdr->first_free_off);
        printf("Student offset %d\n", hdr->first_student_off);
    }
    if (hdr->magic != FILE_MAGIC) {
        fprintf(stderr, "BAD FILE MAGIC: %x", hdr->magic);
        error_found = true;
    }
    uint32_t next_free_offset = hdr->first_free_off;
    struct list_node *head = NULL;
    uint32_t offset = sizeof(*hdr);
    size_t total_size = sizeof *hdr;
    while (offset < LIST_FILESIZE) {
        struct entry_s *pe = offset2addr(hdr, offset);
        if (dump) {
            printf("------------------\n");
            printf("    offset = %d\n", offset);
            printf("    magic = %4.4hx\n", pe->magic);
            printf("    size = %d\n", pe->size);
            printf("    next = %d\n", pe->next_offset);
            if (pe->magic == STUDENT_MAGIC) {
                printf("    id = %d\n", pe->student.id);
                printf("    name = %s\n", pe->student.name);
            }
        }
        if (pe->size == 0) {
            printf("stopping due to a zero length size\n");
            error_found = true;
            break;
        }
        total_size += pe->size;
        switch (pe->magic) {
            case FREE_MAGIC:
                if (offset != next_free_offset) {
                    fprintf(stderr, "NEXT FREE OFFSET IS %d not %d\n", next_free_offset, offset);
                    error_found = true;
                }
                next_free_offset = pe->next_offset;
                if (next_free_offset == offset + pe->size) {
                    fprintf(stderr, "OFFSET %d SHOULD HAVE COALESCED WITH %d\n", offset, next_free_offset);
                    error_found = true;
                }
                break;
            case STUDENT_MAGIC:
                add_to_list(&head, pe, offset);
                size_t max_allocation = strlen(pe->student.name) + 2 * sizeof(*pe);
                if (max_allocation < pe->size) {
                    fprintf(stderr, "OVERALLOCATED ENTRY. MAX NEEDED %ld allocated %hd\n", max_allocation, pe->size);
                    error_found = true;
                }
                break;
            default:
                fprintf(stderr, "UNKNOWN MAGIC %hx at offset %d", pe->magic, offset);
                error_found = true;
                break;
        }
        offset += pe->size;
    }
    if (total_size != LIST_FILESIZE) {
        fprintf(stderr, "FILE HEADER + ENTRIES ARE %ld IN SIZE. EXPECTED %ld\n", total_size, LIST_FILESIZE);
        error_found = true;
    }
    if (!error_found) printf("✅ no errors in the entries\n");
    if (hdr->first_student_off != 0 && head == NULL) {
        fprintf(stderr, "STUDENT LIST NOT EMPTY, BUT DID NOT FIND STUDENT RECORDS\n");
        error_found = true;
    }
    if (head) {
        if (hdr->first_student_off == 0) {
            fprintf(stderr, "STUDENT LIST EMPTY, BUT FOUND STUDENT RECORDS\n");
            error_found = true;
        }
        uint32_t next_student_offset = hdr->first_student_off;
        while (head) {
            if (next_student_offset != head->offset) {
                fprintf(stderr, "EXPECTING NEXT STUDENT AT OFFSET %d FOUND AT %d\n", next_student_offset,
                        head->offset);
                error_found = true;
            }
            next_student_offset = head->entry->next_offset;
            head = head->next;
        }
    }
    if (!error_found) printf("✅ no errors in the student list\n");
    if (error_found) exit(2);
    exit(0);
}
