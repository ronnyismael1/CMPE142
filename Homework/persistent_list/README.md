# Persistent Linked List in C

This project is a robust implementation of a persistent linked list in C, demonstrating proficiency in low-level programming and system calls. It showcases the use of memory-mapped files for persistence, a technique that provides both performance benefits and direct control over file I/O.

## Key Features

- **Memory-Mapped Files**: The project uses the `mmap` system call to map files into memory, allowing for efficient and direct manipulation of data.
- **File I/O**: The code demonstrates proficient use of file operations such as `open`, `fstat`, and `ftruncate`.
- **Dynamic Memory Management**: The project uses dynamic memory allocation (`malloc`) and deallocation (`munmap`), showcasing understanding of memory management in C.
- **Linked List Data Structure**: The project implements a linked list data structure from scratch, demonstrating knowledge of fundamental data structures.
- **Error Handling**: The code includes comprehensive error handling, using functions like `perror` to provide meaningful error messages.
- **Command Line Interface (CLI)**: The project is interactively controlled via command line arguments, demonstrating the creation of a user-friendly CLI.
- **Test Suite**: The project includes a test suite (`dump_plist.c`), demonstrating the importance of testing in software development.

## Usage

```c
TESTING USAGE: ./dump_plist check|dump plist_file

The main program can be run with the following command:

```c
./plist [-t] filename

The -t flag is optional and is used for testing purposes.

Once the program is running, you can use the following commands:

l: List all the ids and students in the list.
a id student_name: Add a student with the given id and name.
d id: Delete the student with the given id.
f id: Find the student with the given id.