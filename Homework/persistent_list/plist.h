#pragma once
#include <stdint.h>
#include <stdbool.h>

// a persistent list that is maintained in id sorted order.
// the free list for the persistent file is maintained in offset sorted order.

// magic for the file_hdr
const uint32_t FILE_MAGIC = 0x5453494c;
// magic for the free space
#define FREE_MAGIC 0xf233
// magic for a student entry
#define STUDENT_MAGIC 0x005d

const size_t LIST_FILESIZE = 64 * 1024;

struct file_hdr {
    uint32_t magic;
    // the offset to the first student in the list
    uint32_t first_student_off;
    // the offset to the first free space
    uint32_t first_free_off;
};

struct student_s {
    int id;
    // null terminated string
    char name[];
};

struct entry_s {
    // size includes the entry_s structure
    uint16_t size;
    uint16_t magic;
    uint32_t next_offset;
    union {
        struct student_s student;
        char filler[0];
    };
};

// returns the address of the byte offset bytes from the beginning of the file at address hdr
void *offset2addr(struct file_hdr *hdr, uint32_t offset) {
    return offset == 0 ? NULL : ((char *)hdr) + offset;
}