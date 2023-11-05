```markdown
# Simple Smart ELF Loader Documentation

**Group 4**
**November 5, 2023**

## 1. Introduction

This document provides documentation for the Simple Smart ELF Loader written in C. The program is designed to load and execute ELF files, handling segmentation faults and memory management.

## 2. Code Explanation

### 2.1 Global Variables


```c
Elf32_Ehdr *elf_header;        // ELF header
int file_descriptor;            // File descriptor
int system_page_size;          // System page size
int page_fault_count = 0;
int page_allocation_count = 0;
float total_internal_fragmentation = 0.0f;
```

These global variables store information about the ELF file, system page size, and various statistics related to memory management and segmentation faults.

### 2.2 Signal Handler

```c
void segmentation_fault_handler(int signal_number, siginfo_t *signal_info, void *context) {
    // ...
}
```

This function is the signal handler for segmentation faults. It attempts to allocate memory for the faulting address within a PT LOAD segment, loads the segment data, and updates statistics. If the address doesn't belong to any segment, it reports an invalid memory access.

### 2.3 Main Function

```c
int main(int argc, char **argv) {
    // ...
}
```

The 'main' function sets up the signal handler, opens the ELF file, reads the ELF header, and attempts to execute the 'start' method. It also prints the entry point address and reports statistics.

## 3. Compile and Run

To compile and run the Simple Smart ELF Loader, follow these steps:

```shell
$ make
$ ./loader given_file
```

Replace ```given_file``` with the path to the ELF executable you want to load.

## 4. Contribution

**Gagan's Contribution:**

Handled the setup of the code, including setting up the signal handler, opening the ELF file, reading the ELF header, managing the initial setup and executing the 'start' method.

**Garv's Contribution:**

Garv's contribution focused on managing the cleanup process, printing the entry point address, and reporting statistics on page faults and internal fragmentation.

## 5. Conclusion

The Simple Smart ELF Loader is a versatile tool for loading and executing ELF executables while managing memory and handling segmentation faults.
```
