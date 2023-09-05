<div align="center">
  <h1 style="font-size: 50px;">ELF Loader</h1>
</div>
Welcome to the ELF Loader, a program designed to load and execute ELF (Executable and Linkable Format) files. This README provides a concise overview of the loader's functionality and usage.

To use the ELF loader, follow these steps:

1. Compile the loader.c source code using a C compiler. You can use `gcc`:

   ```bash
   gcc loader.c -o loader
   ```

2. Run the compiled `loader` program with the path to the ELF executable as a command-line argument:

   ```bash
   ./loader.c any_file
   ```
   `any_file` is file you can change to execute the functioning of the loader.

3. The loader will load the ELF file into memory, execute it, and print the return value from the `_start` method (if present) to the console.

## Code Explanation

The loader.c code can be explained as follows:

1. **Load entire binary content into memory from the ELF file (Line 22-38):** The `load_and_run_elf` function starts by opening the specified ELF executable file and reading its ELF header, which contains crucial information about the file's organization.

2. **Iterate through the program headers and find PT_LOAD segments (Line 39-84):** The code iterates through the program headers, allocating memory for each program header, and checking if it's a loadable segment (PT_LOAD). If it's a loadable segment, the code allocates memory using `mmap` and copies the segment's content into memory.

3. **Navigate to the entry point address and execute (Line 81-84):** The loader calculates the entry point address by adding the virtual address of the segment to the memory address. Then, it executes the loaded ELF program.

4. **Typecast and call "_start" method, then print the returned value (Line 86-90):** The code typecasts the entry point address to match the signature of the `_start` method in the loaded ELF program and calls it. It prints the returned value to the console.

5. **Clean up resources (Line 92-103):** After execution, the allocated memory for the segment is released using `munmap` to free up resources. Memory for program headers, ELF headers, and file descriptors is also freed.

6. **Carry out necessary checks on the input ELF file (Line 107-111):** In the `main` function, the code checks whether the correct number of command-line arguments is provided. If not, it prints an error message indicating the program's usage and exits with an error code.

7. **Pass the ELF executable to the loader for loading and execution (Line 114-118):** The `load_and_run_elf` function is called with the ELF executable file name provided as a command-line argument, initiating the loading and execution process.

8. **Invoke the cleanup routine inside the loader (Line 120-125):** After execution is complete, the `loader_cleanup` function is invoked to perform any necessary cleanup operations.

9. **Main function returns success (Line 126):** The `main` function returns 0 to indicate successful execution.

## Conclusion


The ELF Loader is your go-to tool for effortlessly running ELF (Executable and Linkable Format) files. It takes the complexity out of executing ELF programs, making it super handy for various tasks that involve running these types of files.

This tool is all about simplicity. You compile it, run it, and boom! It loads and runs your ELF program without any fuss. Plus, it cleans up after itself, so you don't have to worry about lingering messes in your system.

Whether you're a developer, a tester, or just someone curious about ELF files, the ELF Loader has got your back. It's designed to make your life easier when working with these binaries.

So, why make things complicated? Try out the ELF Loader and simplify your interactions with ELF executables today!
