// #include "loader.h"

// Elf32_Ehdr *ehdr;
// Elf32_Phdr *phdr;
// int fd;
// typedef void (*EntryFunction)();
// /*
// * Release memory and perform other cleanups
// */
// void loader_cleanup() {
//     // Add cleanup logic if needed
// }

// /*
// * Load and run the ELF executable file
// */
// void load_and_run_elf(char* exe) {
//     // 1. Load entire binary content into the memory from the ELF file.
//     fd = open(exe, O_RDONLY);

//     if (fd == -1) {
//         perror("open");
//         exit(1);
//     }

//     // Read the ELF header
//     ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
//     if (!ehdr) {
//         perror("malloc");
//         close(fd);
//         exit(1);
//     }

//     if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
//         perror("read");
//         free(ehdr);
//         close(fd);
//         exit(1);
//     }

//     // 2. Iterate through the program headers
//     for (int i = 0; i < ehdr->e_phnum; i++) {
//         phdr = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr));
//         if (!phdr) {
//             perror("malloc");
//             close(fd);
//             free(ehdr);
//             exit(1);
//         }

//         if (lseek(fd, ehdr->e_phoff + i * ehdr->e_phentsize, SEEK_SET) == -1) {
//             perror("lseek");
//             free(phdr);
//             close(fd);
//             free(ehdr);
//             exit(1);
//         }

//         if (read(fd, phdr, sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)) {
//             perror("read");
//             free(phdr);
//             close(fd);
//             free(ehdr);
//             exit(1);
//         }

//         // Find the PT_LOAD segment
//         if (phdr->p_type == PT_LOAD) {
//             // 3. Allocate memory of the size "p_memsz" using mmap function and then copy the segment content
//             void *virtual_mem = mmap(
//                 NULL, phdr->p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC,
//                 MAP_ANONYMOUS | MAP_PRIVATE, 0, 0
//             );

//             if (virtual_mem == MAP_FAILED) {
//                 perror("mmap");
//                 free(phdr);
//                 close(fd);
//                 free(ehdr);
//                 exit(1);
//             }

//             if (lseek(fd, phdr->p_offset, SEEK_SET) == -1) {
//                 perror("lseek");
//                 munmap(virtual_mem, phdr->p_memsz);
//                 free(phdr);
//                 close(fd);
//                 free(ehdr);
//                 exit(1);
//             }

//             if (read(fd, virtual_mem, phdr->p_filesz) != phdr->p_filesz) {
//                 perror("read");
//                 munmap(virtual_mem, phdr->p_memsz);
//                 free(phdr);
//                 close(fd);
//                 free(ehdr);
//                 exit(1);
//             }

//             // Execute the ELF binary by jumping to the entry point
//             EntryFunction entry_point = (EntryFunction)(uintptr_t)ehdr->e_entry;
//             entry_point();

//             // Clean up
//             munmap(virtual_mem, phdr->p_memsz);
//             free(phdr);
//             close(fd);
//             free(ehdr);
//             return;
//         } 

        
//         free(phdr);
//     }

//     // Clean up
//     close(fd);
//     free(ehdr);
//     fprintf(stderr, "No PT_LOAD segment found.\n");
//     exit(1);
// }

// int main(int argc, char** argv) {
//     if (argc != 2) {
//         // 1. carry out necessary checks on the input ELF file
//         fprintf(stderr, "Usage: %s \n", argv[0]);
//         exit(1);
//     }

//     // 2. passing it to the loader for carrying out the loading /execution
//     load_and_run_elf(argv[1]);

//     // 3. invoke the cleanup routine inside the loader
//     loader_cleanup();
//     return 0;
// }

// #include "loader.h"
// #include <signal.h>

// Elf32_Ehdr *ehdr;
// int fd;
// int page_size;
// int page_faults = 0;
// int page_allocations = 0;
// float internal_fragmentation = 0.0f; // Change to float
// typedef void (*EntryFunction)();

// // Signal handler for segmentation faults
// void segfault_handler(int signo, siginfo_t *si, void *ctx) {
//     void *addr = si->si_addr;
    
//     // Check if the address that caused the segmentation fault is within a PT_LOAD segment
//     for (int i = 0; i < ehdr->e_phnum; i++) {
//         Elf32_Phdr phdr;
//         if (lseek(fd, ehdr->e_phoff + i * ehdr->e_phentsize, SEEK_SET) == -1) {
//             perror("lseek");
//             exit(1);
//         }

//         if (read(fd, &phdr, sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)) {
//             perror("read");
//             exit(1);
//         }

//         if (phdr.p_type == PT_LOAD && 
//             (uintptr_t)addr >= phdr.p_vaddr && 
//             (uintptr_t)addr < phdr.p_vaddr + phdr.p_memsz) {
//             // Allocate memory for the segment containing the faulting address
//             int offset = phdr.p_offset & ~(page_size - 1); // Align to page boundary
//             int size = ((phdr.p_memsz + (page_size - 1)) & ~(page_size - 1));
//             void *mem = mmap((void*)phdr.p_vaddr, size, PROT_READ | PROT_WRITE | PROT_EXEC,
//                              MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, offset);
//             if (mem == MAP_FAILED) {
//                 perror("mmap");
//                 exit(1);
//             }
            
//             // Load the segment data into the allocated memory
//             if (lseek(fd, phdr.p_offset, SEEK_SET) == -1) {
//                 perror("lseek");
//                 exit(1);
//             }

//             if (read(fd, mem, phdr.p_filesz) != phdr.p_filesz) {
//                 perror("read");
//                 exit(1);
//             }

//             page_faults++;
//             page_allocations++;
//     // Calculate internal fragmentation as a float
//             float internal_fragmentation_float = (float) (size - phdr.p_memsz) / 1024.0f;
//             internal_fragmentation += internal_fragmentation_float;

//             return;
//         }
//     }
//     // If the address doesn't belong to any PT_LOAD segment, it's an invalid access
//     fprintf(stderr, "Segmentation fault: Invalid memory access at %p\n", addr);
//     exit(1);
// }

// int main(int argc, char** argv) {
//     if (argc != 2) {
//         fprintf(stderr, "Usage: %s <ELF executable>\n", argv[0]);
//         exit(1);
//     }

//     // Set up the segmentation fault handler
//     struct sigaction sa;
//     sa.sa_flags = SA_SIGINFO;
//     sa.sa_sigaction = segfault_handler;
//     sigaction(SIGSEGV, &sa, NULL);

//     // Open the ELF file
//     fd = open(argv[1], O_RDONLY);
//     if (fd == -1) {
//         perror("open");
//         exit(1);
//     }

//     // Read the ELF header
//     ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
//     if (!ehdr) {
//         perror("malloc");
//         close(fd);
//         exit(1);
//     }

//     if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
//         perror("read");
//         free(ehdr);
//         close(fd);
//         exit(1);
//     }

//     // Get the system page size
//     page_size = getpagesize();

//     // Attempt to execute the _start method
//     EntryFunction entry_point = (EntryFunction)(uintptr_t)ehdr->e_entry;
//     entry_point();

//     // Clean up
//     close(fd);
//     free(ehdr);

//     // Report statistics
//     printf("Total page faults: %d\n", page_faults);
//     printf("Total page allocations: %d\n", page_allocations);
//     // Report total internal fragmentation as a float
//     printf("Total internal fragmentation (KB): %.2f\n", internal_fragmentation);

//     return 0;
// }

#include "loader.h"
#include <signal.h>

Elf32_Ehdr *elf_header; // ELF header
int file_descriptor; // File descriptor
int system_page_size; // System page size
int page_fault_count = 0;
int page_allocation_count = 0;
float total_internal_fragmentation = 0.0f;
typedef void (*EntryFunction)();

// Signal handler for segmentation faults
void segmentation_fault_handler(int signal_number, siginfo_t *signal_info, void *context) {
    void *faulting_address = signal_info->si_addr;
    
    // Check if the faulting address is within a PT_LOAD segment
    for (int i = 0; i < elf_header->e_phnum; i++) {
        Elf32_Phdr program_header;
        if (lseek(file_descriptor, elf_header->e_phoff + i * elf_header->e_phentsize, SEEK_SET) == -1) {
            perror("lseek");
            exit(1);
        }

        if (read(file_descriptor, &program_header, sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)) {
            perror("read");
            exit(1);
        }

        if (program_header.p_type == PT_LOAD && 
            (uintptr_t)faulting_address >= program_header.p_vaddr && 
            (uintptr_t)faulting_address < program_header.p_vaddr + program_header.p_memsz) {
            // Allocate memory for the segment containing the faulting address
            int offset = program_header.p_offset & ~(system_page_size - 1); // Align to page boundary
            int size = ((program_header.p_memsz + (system_page_size - 1)) & ~(system_page_size - 1));
            void *memory = mmap((void*)program_header.p_vaddr, size, PROT_READ | PROT_WRITE | PROT_EXEC,
                             MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, offset);
            if (memory == MAP_FAILED) {
                perror("mmap");
                exit(1);
            }
            
            // Load the segment data into the allocated memory
            if (lseek(file_descriptor, program_header.p_offset, SEEK_SET) == -1) {
                perror("lseek");
                exit(1);
            }

            if (read(file_descriptor, memory, program_header.p_filesz) != program_header.p_filesz) {
                perror("read");
                exit(1);
            }

            page_fault_count++;
            page_allocation_count++;
            // Calculate internal fragmentation as a float
            float internal_fragmentation_float = (float) (size - program_header.p_memsz) / 1024.0f;
            total_internal_fragmentation += internal_fragmentation_float;

            return;
        }
    }
    // If the address doesn't belong to any PT_LOAD segment, it's an invalid access
    fprintf(stderr, "Segmentation fault: Invalid memory access at %p\n", faulting_address);
    exit(1);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ELF executable>\n", argv[0]);
        exit(1);
    }

    // Set up the segmentation fault handler
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = segmentation_fault_handler;
    sigaction(SIGSEGV, &sa, NULL);

    // Open the ELF file
    file_descriptor = open(argv[1], O_RDONLY);
    if (file_descriptor == -1) {
        perror("open");
        exit(1);
    }

    // Read the ELF header
    elf_header = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    if (!elf_header) {
        perror("malloc");
        close(file_descriptor);
        exit(1);
    }

    if (read(file_descriptor, elf_header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("read");
        free(elf_header);
        close(file_descriptor);
        exit(1);
    }

    // Get the system page size
    system_page_size = getpagesize();

    // Attempt to execute the _start method
    EntryFunction entry_point = (EntryFunction)(uintptr_t)elf_header->e_entry;
    entry_point();

    // Clean up
    close(file_descriptor);
    free(elf_header);

    // Print the entry point address
    printf("Entry Point Address: 0x%x\n", elf_header->e_entry);

    // Report statistics
    printf("Total page faults: %d\n", page_fault_count);
    printf("Total page allocations: %d\n", page_allocation_count);
    // Report total internal fragmentation as a float
    printf("Total internal fragmentation (KB): %.2f\n", total_internal_fragmentation);

    return 0;
}

