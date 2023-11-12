# pbiased_bits

This is a C program that analyzes binary data from multiple files. The program reads binary data line by line from each file, validates the data, and calculates the percentage of set bits (1s) in each pair of lines. The program keeps track of the lowest and highest percentages of set bits across all files.

## Features

- Reads and validates binary data from multiple files
- Calculates the percentage of set bits in each pair of lines
- Tracks the lowest and highest percentages of set bits across all files
- Handles multiple files concurrently using child processes
- Communicates results from child processes to parent process through a pipe

## Usage

1. Compile the program using the provided Makefile: `make`
2. Run the program with the names of the binary files as arguments: `./pbiased_bits file1 file2 ...`

## Testing

Unit tests are provided for key functions in the program. To run the tests:

1. Compile the test program using the provided Makefile: `make test`
2. Run the test program: `./test_update_percentage`

## Requirements

- GCC
- Make
- CUnit