#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

/*
* Release memory and perform other cleanups
*/
void loader_cleanup() {
     if (fd != -1) {
        close(fd);
    }
    if (ehdr != NULL) {
        free(ehdr);
    }
}

/*
* Load and run the ELF executable file
*/
void load_and_run_elf(char* exe) {
    // 1. Load entire binary content into the memory from the ELF file.
    fd = open(exe, O_RDONLY);

    if (fd == -1) {
        perror("open");
        exit(1);
    }

    // Read the ELF header
    ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    if (!ehdr) {
        perror("malloc");
        close(fd);
        exit(1);
    }

    if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("read");
        free(ehdr);
        close(fd);
        exit(1);
    }

    // 2. Iterate through the program headers
    for (int i = 0; i < ehdr->e_phnum; i++) {
        phdr = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr));
        if (!phdr) {
            perror("malloc");
            close(fd);
            free(ehdr);
            exit(1);
        }

        if (lseek(fd, ehdr->e_phoff + i * ehdr->e_phentsize, SEEK_SET) == -1) {
            perror("lseek");
            free(phdr);
            close(fd);
            free(ehdr);
            exit(1);
        }

        if (read(fd, phdr, sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)) {
            perror("read");
            free(phdr);
            close(fd);
            free(ehdr);
            exit(1);
        }

        // Find the PT_LOAD segment
        if (phdr->p_type == PT_LOAD) {
        // 3. Allocate memory of the size "p_memsz" using mmap function and then copy the segment content
           
          void *virtual_mem = mmap(
                NULL, phdr->p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_ANONYMOUS | MAP_PRIVATE, 0, 0
                );

            if (virtual_mem == MAP_FAILED) {
                perror("mmap");
                free(phdr);
                close(fd);
                free(ehdr);
                exit(1);
            }

            if (lseek(fd, phdr->p_offset, SEEK_SET) == -1) {
                perror("lseek");
                munmap(virtual_mem, phdr->p_memsz);
                free(phdr);
                close(fd);
                free(ehdr);
                exit(1);
            }

            if (read(fd, virtual_mem, phdr->p_filesz) != phdr->p_filesz) {
                perror("read");
                munmap(virtual_mem, phdr->p_memsz);
                free(phdr);
                close(fd);
                free(ehdr);
                exit(1);
            }

    
            //4. Get the entry point address
            int (*entry_point)() = (int (*)())(phdr->p_vaddr + (uintptr_t)virtual_mem);
          
            //5. Capture the return value from the entry point 
            int result = entry_point();
            
            // 6. Call the "_start" method and print the value returned from the "_Start"
            printf("User _start return value = %d\n", result);

            // Clean up
            munmap(virtual_mem, phdr->p_memsz);
            free(phdr);
            close(fd);
            free(ehdr);
            return;
        }

        free(phdr);
    }

    // Again Clean up
    close(fd);
    free(ehdr);
    fprintf(stderr, "No PT_LOAD segment found.\n");
    exit(1);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        // 1.Necessary checks on the input ELF file
        fprintf(stderr, "Usage: %s \n", argv[0]);
        exit(1);
    }

    // 2. Passing on to loader for carrying out execution
    load_and_run_elf(argv[1]);

    // 3. Invoke the cleanup routine inside the loader
    loader_cleanup();
    return 0;
}
