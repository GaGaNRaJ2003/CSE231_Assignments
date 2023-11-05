#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include "dummy_main.h"

// Constants and definitions

#define RESET "\033[0m"
#define BOLD_YLW "\033[1m\033[33m" /* Bold Yellow */
#define BOLD_BLU "\033[1m\033[34m" /* Bold Blue */
#define BOLD_MAG "\033[1m\033[35m" /* Bold Magenta */
#define BOLD_CYN "\033[1m\033[36m" /* Bold Cyan */
#define ANSI_COLOR_LIGHT_GREEN "\x1b[92m"
#define BOLD_ORG "\033[1m\033[33m" /* Bold Orange */
#define ANSI_COLOR_LIGHT_RED "\x1b[91m"
#define BOLD_FUS "\033[1m\033[31m" /* Bold Fuchsia */

#define Line_Buffer_Size 1280
#define token_Buffer_Size 80
#define HISTORY_SIZE 150

char *history[HISTORY_SIZE];
int count_history = 0;
#define DEFAULT_TSLICE 1000 // 1 second in milliseconds

int NCPU = 1;        // Number of CPU resources
int TSLICE = DEFAULT_TSLICE; // Time quantum for each process

char *previous_command = NULL;

typedef struct {
    int tid; // Task ID
    int priority; // Priority of the task
    pthread_t thread; // Thread to execute the task
    char *command; // Command to execute
    // Add any other task-related data here
} Task;

// Function prototypes
char *lsh_line_reading(void);
char **lsh_line_spliting(char *line);
int lsh_systemCall(char **arguments);
int echo_systemCall(char **arguments);
int ls_systemCall(char **arguments);
int grep_systemCall(char **arguments);
int wc_systemCall(char **arguments);

volatile sig_atomic_t background_process_count = 0;

#define MAX_NCPU 32


void manage_history(const char *command) {
    //Storing a copy of the command string
    if (count_history < HISTORY_SIZE) {
        history[count_history] = strdup(command);
        count_history++;
    } else {
        // in case of buffer overflow , deleting first member of history array
        free(history[0]);
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            history[i] = history[i + 1];
        }
        history[149] = strdup(command);
    }
    if (previous_command != NULL) {
        free(previous_command);
    }
    previous_command = strdup(command);
}

void print_history() {
    int i;
    int status;
    pid_t wpid;

    for (i = 0; i < count_history; i++) {
        pid_t process_id;
        struct timeval t0_time, end_t;

        gettimeofday(&t0_time, NULL);
        process_id = fork();

        if (process_id == 0) {
            char **arguments = lsh_line_spliting(history[i]);
            lsh_systemCall(arguments);
            free(arguments);
            exit(EXIT_SUCCESS);
        } else if (process_id < 0) {
            perror("lsh");
        } else {
            if (waitpid(process_id, &status, 0) > 0) {
                gettimeofday(&end_t, NULL);
                double execution_time = (end_t.tv_sec - t0_time.tv_sec) +
                                        (end_t.tv_usec - t0_time.tv_usec) / 1000000.0;

                printf("%s%d%s: %s%s%s\n", BOLD_ORG, i + 1, RESET,
                       ANSI_COLOR_LIGHT_GREEN, history[i], ANSI_COLOR_LIGHT_RED);
                printf("  Process ID: %d\n", process_id);
                printf("  Execution Time: %.6f seconds\n", execution_time);
            }
        }
    }
}


// 


void background_process_handler(int signum) {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        background_process_count--;
    }
}


int lsh_launch(char **arguments) {
    pid_t process_id;
    int status;
    // for "&" command
    int flag_bg_process = 0, i = 0;

    do {
        i++;
    } while (arguments[i] != NULL);
    if (strcmp(arguments[i - 1], "&") == 0 && i > 0) {
        flag_bg_process = 1;
        arguments[i - 1] = NULL; // "&" Removed from arguments
    }

    struct timeval t0_time, end_t;
    gettimeofday(&t0_time, NULL); // Get the start time

    process_id = fork();
    if (process_id == 0) {
        // in the child process

        if (execvp(arguments[0], arguments) == -1) {
            // execvp fails
            perror("lsh");
            exit(EXIT_FAILURE);
        }
    }  
    if (process_id < 0) {
        // fork() fails
        perror("lsh");
    } else {
        if (flag_bg_process) {
            signal(SIGCHLD, background_process_handler);
            background_process_count++;
        } else {
            waitpid(process_id, &status, 0);
            if (WIFEXITED(status)) {
                gettimeofday(&end_t, NULL); // Get the end time
                double execution_time = (end_t.tv_sec - t0_time.tv_sec) +
                                        (end_t.tv_usec - t0_time.tv_usec) / 1000000.0;

                printf("Process ID: %d\n", process_id);
                printf("Execution Time: %f seconds\n", execution_time);
            }
        }
    }

    return 1;
}


int lsh_systemCall(char **arguments) {
    int i;

    if (arguments[0] == NULL) {
        // An empty command -> entered
        return 1;
    }

    //  for built-in commands
    if (strcmp(arguments[0], "history") == 0) {
        print_history();
        return 1;
    }
     if (strcmp(arguments[0], "exit") == 0) {
        return 0;
    }

    // Adding command to history_array
    manage_history(arguments[0]);

    //  dealing with specific external commands 
    if (strcmp(arguments[0], "ls") == 0) {
        return ls_systemCall(arguments);
    } 
     if (strcmp(arguments[0], "echo") == 0) {
        return echo_systemCall(arguments);
    } 
     if (strcmp(arguments[0], "wc") == 0) {
        return wc_systemCall(arguments);
    } 
     if (strcmp(arguments[0], "grep") == 0) {
        return grep_systemCall(arguments);
    }

    return lsh_launch(arguments); // dealing with any other external commands
}

int wc_systemCall(char **arguments) {
    pid_t process_id, wprocess_id;
    int status;

    process_id = fork();
    if (process_id == 0) {
        // In the child process, redirect stdin from /dev/null
        if (freopen("/dev/null", "r", stdin) == NULL) {
            perror("freopen");
            exit(EXIT_FAILURE);
        }

        if (execvp(arguments[0], arguments) == -1) {
            // execvp fails
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (process_id < 0) {
        perror("lsh");
    } else {
        while (true) {
            wprocess_id = waitpid(process_id, &status, WUNTRACED);
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                break;  // Exit the loop when the child process terminates/exits
            }
        }
    }

    return 1;
}


int ls_systemCall(char **arguments) {
    pid_t process_id, wprocess_id;
    int status;

    process_id = fork();
    if (process_id == 0) {
                // in the child process

        if (execvp(arguments[0], arguments) == -1) {
            // execvp fails
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (process_id < 0) {
        perror("lsh");
    } else {
        while (true) {
    wprocess_id = waitpid(process_id, &status, WUNTRACED);
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
        break;  // Exit the loop when -  child process terminated/exits  
    }
}
    }

    return 1;
}

int grep_systemCall(char **arguments) {
    pid_t process_id, wprocess_id;
    int status;

    process_id = fork();
    if (process_id == 0) {
                // in the child process
        if (execvp(arguments[0], arguments) == -1) {
            // execvp fails
            perror("lsh");

        }
        exit(EXIT_FAILURE);
    } else if (process_id < 0) {
        perror("lsh");
    } else {
      while (true) {
    wprocess_id = waitpid(process_id, &status, WUNTRACED);
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
        break;  // Exit the loop when -  child process terminated/exits  
    }
}}


    return 1;
}

int echo_systemCall(char **arguments) {
    int i = 1;  

    while (arguments[i] != NULL) {
        printf("%s", arguments[i]);
        if (arguments[i + 1] != NULL) {
            printf(" ");
        }
        i++;
    }
    printf("\n");
    return 1;
}

#define MAX_TASKS 100 // Define the maximum number of tasks


// Initialize an index to track the highest priority task
int highest_priority_index = -1;

// Check if the priority queue is empty
int priority_queue_is_empty() {
    return highest_priority_index == -1;
}
Task task_pool[MAX_NCPU]; // Array of tasks for each CPU

// Get the highest priority task
Task *get_highest_priority_task() {
    if (highest_priority_index == -1) {
        return NULL;
    }
    Task *task = &task_pool[highest_priority_index];
    highest_priority_index = -1; // Reset the index
    return task;
}

// Add a task back to the priority queue (task_pool)
void add_task_back_to_priority_queue(Task *task) {
    if (task == NULL) {
        return;
    }
    
    // Find an empty slot in the task_pool array
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_pool[i].command == NULL) {
            task_pool[i] = *task;
            // Update the highest_priority_index if needed
            if (highest_priority_index == -1 || task->priority < task_pool[highest_priority_index].priority) {
                highest_priority_index = i;
            }
            break;
        }
    }
}

char *builtin_function[] = {
    "history",
    "exit"
};

int (*builtin_func[])(char **) = {
    &lsh_systemCall,
    &lsh_systemCall
};


int lsh_num_builtins() {
    return sizeof(builtin_function) / sizeof(char *);
}

/*int main(int argc, char **argv) {
    printf("%sWelcome to %sAggaris!%s%s\n",BOLD_MAG, BOLD_MAG, BOLD_YLW,RESET);
    printf("%s\t\tCommands: ls, echo, history, pwd, exit, cat, grep, date, sort, uniq, wc\n", BOLD_FUS);

    if (argc == 2) {
        // code for script file
        FILE *file_s = fopen(argv[1], "r");
        if (file_s == NULL) {
            perror("Error occured : ");
            return EXIT_FAILURE;
        }
        ssize_t read;
        char *line = NULL;
        size_t length = 0;
        
        while ((read = getline(&line, &length, file_s)) != -1) {
            // Processing and executing -> script file line by line 
            char **arguments = lsh_line_spliting(line);
            lsh_systemCall(arguments);
            free(arguments);
            free(line);
            
        }
        fclose(file_s);
    }
    else if (argc != 2){
    char *line;
    char **arguments;
    int status;

   char cwd[4096]; // Buffer for current directory
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        do {
            printf("%s%s%s%s %s%s%s%s%s ", BOLD_BLU , "Aggaris:", RESET,
                BOLD_CYN, cwd, RESET,
                BOLD_YLW, "$", RESET); // Prompt with shell name, current directory, and $
            line = lsh_line_reading();
            arguments = lsh_line_spliting(line);
            status = lsh_systemCall(arguments);

            free(line);
            free(arguments);
        } while (status);
    } else {
        perror("getcwd");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
    }
}*/


char *lsh_line_reading(void) {
    char *line = NULL;
    ssize_t bufferSize = 0;

    ssize_t read = getline(&line, &bufferSize, stdin);
    if (read == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        } else {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }

    return line;
}


char **lsh_line_spliting(char *line) {
    int bufferSize = token_Buffer_Size;
    int pos = 0;
    char **pointer_token = malloc(bufferSize * sizeof(char *));
    char *token;

     if (!pointer_token) {
        perror("lsh: allocation error");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " \t\r\n\a");
    while (token != NULL) {
        if (strcmp(token, "&") == 0 && pos > 0) {
            pointer_token[pos] = NULL;
            break; //   '&' is encountered -> exit from loop
        }
        pointer_token[pos] = token;
        pos++;

        if (pos >= bufferSize) {
            bufferSize += token_Buffer_Size;
            pointer_token = realloc(pointer_token, bufferSize * sizeof(char *));
            if (!pointer_token) {
                perror("lsh: allocation error");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, " \t\r\n\a");
    }
    pointer_token[pos] = NULL;
    return pointer_token;
} 




/*void manage_history(const char *command) {
    // Storing a copy of the command string
    if (count_history < HISTORY_SIZE) {
        history[count_history] = strdup(command);
        count_history++;
    } else {
        // In case of buffer overflow, delete the first member of the history array
        free(history[0]);
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            history[i] = history[i + 1];
        }
        history[149] = strdup(command);
    }
    if (previous_command != NULL) {
        free(previous_command);
    }
    previous_command = strdup(command);
}*/




pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t task_ready = PTHREAD_COND_INITIALIZER;

// Function to execute a task (placeholder for actual implementation)
void execute_task(Task *task) {
    if (task->command != NULL) {
        printf("Executing task (Thread ID: %d, Priority: %d): %s\n", task->tid, task->priority, task->command);

        // Execute the task's command
        int status = system(task->command);

        if (WIFEXITED(status)) {
            printf("Task (Thread ID: %d) exited with status %d\n", task->tid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Task (Thread ID: %d) terminated by signal %d\n", task->tid, WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            printf("Task (Thread ID: %d) stopped by signal %d\n", task->tid, WSTOPSIG(status));
        }
    }
}

// Task management functions (to be used by the scheduler)
void add_task(const char *command, int priority) {
    // Find an available task in task_pool
    Task *task = NULL;
    pthread_mutex_lock(&task_mutex);
    for (int i = 0; i < NCPU; i++) {
        if (task_pool[i].command == NULL) {
            task = &task_pool[i];
            break;
        }
    }
    if (task != NULL) {
        task->priority = priority;
        task->command = strdup(command);
    }
    pthread_cond_signal(&task_ready);
    pthread_mutex_unlock(&task_mutex);
}


void *task_thread(void *arg) {
    Task *task = (Task *)arg;
    while (1) {
        pthread_mutex_lock(&task_mutex);
        while (task->command == NULL) {
            pthread_cond_wait(&task_ready, &task_mutex);
        }
        pthread_mutex_unlock(&task_mutex);
        execute_task(task);
        free(task->command);
        task->command = NULL;
    }
    return NULL;
}

typedef struct {
    pid_t pid;      // Process ID
    time_t start_time; // Start time
    time_t end_time;   // End time (if finished)
    int status;     // 0 for running, 1 for terminated
} Job;

Job background_jobs[MAX_NCPU]; // Track background jobs for each CPU

void track_background_jobs() {
    int i;
    for (i = 0; i < NCPU; i++) {
        if (background_jobs[i].status == 0) {
            int status;
            pid_t wpid = waitpid(background_jobs[i].pid, &status, WNOHANG);
            if (wpid > 0) {
                background_jobs[i].end_time = time(NULL);
                background_jobs[i].status = 1;
                printf("Background Job (PID %d) completed.\n", background_jobs[i].pid);
            }
        }
    }
}
// Scheduling logic
int current_task_index = 0;

void round_robin_scheduler() {
    while (1) {
        // Implement priority-based task scheduling here

        // Lock the task_mutex to access the priority queue
        pthread_mutex_lock(&task_mutex);

        // Check if there are tasks in the priority queue
        if (priority_queue_is_empty()) {
            // If the queue is empty, release the lock and wait for tasks
            pthread_mutex_unlock(&task_mutex);
            usleep(100000); // Sleep for a short time before checking again
        } else {
            // Get the highest priority task from the priority queue
            Task *task = get_highest_priority_task();

            // Release the lock and signal the task to start running
            pthread_mutex_unlock(&task_mutex);

            // Signal the task to start running
            pthread_cond_signal(&task_ready);

            // Sleep for the time quantum (TSLICE)
            usleep(TSLICE * 1000); // TSLICE is in milliseconds, so convert to microseconds

            // Lock the task_mutex again to add the task back to the queue
            pthread_mutex_lock(&task_mutex);

            // Add the task back to the priority queue
            add_task_back_to_priority_queue(task);

            // Release the lock
            pthread_mutex_unlock(&task_mutex);
        }
    }
}



void initialize_scheduler() {
    // Initialize the scheduler data structures and threads here
    // You can create and manage a thread pool for process scheduling

    for (int i = 0; i < NCPU; i++) {
        task_pool[i].tid = i;
        task_pool[i].command = NULL;
        pthread_create(&(task_pool[i].thread), NULL, task_thread, &(task_pool[i]));
    }
}







int main(int argc, char **argv) {
    // Check if the number of command-line arguments is correct
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <NCPU> <TSLICE>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Parse command-line arguments for NCPU and TSLICE
    NCPU = atoi(argv[1]);
    TSLICE = atoi(argv[2]);

    if (NCPU <= 0 || NCPU > MAX_NCPU || TSLICE <= 0) {
        fprintf(stderr, "Invalid NCPU or TSLICE values.\n");
        return EXIT_FAILURE;
    }

    printf("%sWelcome to %sAggaris!%s%s\n", BOLD_MAG, BOLD_MAG, BOLD_YLW, RESET);
    printf("%s\t\tCommands: ls, echo, history, pwd, exit, cat, grep, date, sort, uniq, wc\n", BOLD_FUS);

    // Initialize the scheduler
    initialize_scheduler(NCPU);

    char *line;
    char **arguments;
    int status;

    char cwd[4096]; // Buffer for the current directory
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        do {
            printf("%s%s%s%s %s%s%s%s%s ", BOLD_BLU, "Aggaris:", RESET,
                BOLD_CYN, cwd, RESET,
                BOLD_YLW, "$", RESET); // Prompt with shell name, current directory, and $
            line = lsh_line_reading();
            arguments = lsh_line_spliting(line);
            status = lsh_systemCall(arguments);

            free(line);
            free(arguments);
        } while (status);
    } else {
        perror("getcwd");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}