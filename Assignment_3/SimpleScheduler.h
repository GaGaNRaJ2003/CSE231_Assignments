#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>

#define DEFAULT_TSLICE 1000 // MILLISECOND 1 second 
int TSLICE = DEFAULT_TSLICE; //DEFINING Time quantum 
char *last_command  = NULL;
int GivenCpu = 1;       


typedef struct {
     pthread_t thread;// Thread 
    int priority; // TO judge priority
     int tid;
    char *command; // Command 
} Task;


#define MCPU 64

#define MAXIMUM_TASK 150 // Define the maximum number of tasks


// Initialize an index to track the highest priority task
int highest_priority_index = -1;

// empty priority queue  
int check_empty_priority_queue() {
    return highest_priority_index == -1;
}
Task *arrays_of_tasks;
// Get the top priority task
Task *getting_top_priority_task() {
    if (highest_priority_index == -1) {
        return NULL;
    }
    Task *task = &arrays_of_tasks[highest_priority_index];
    highest_priority_index = -1; 
    return task;
}

// function to reduce task priority 
void new_task_addition_back_to_priority_queue(Task *task) {
    if (task == NULL) {
        //task is null , no task so return
        return;
    }
    
    // searching in the arrays_of_tasks for empty slot 
    for (int i = 0; i < MAXIMUM_TASK; i++) {
        if (arrays_of_tasks[i].command == NULL) {
            arrays_of_tasks[i] = *task;
            //here Updating the highest_priority_index 
            if ( arrays_of_tasks[highest_priority_index].priority > task->priority || highest_priority_index == -1 ) {
                highest_priority_index = i;
            }
            break;
        }
    }
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
pthread_mutex_t tmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ReadyTask = PTHREAD_COND_INITIALIZER;

// Function for executing  task 
void executing_task(Task *task) {
    if (task->command != NULL) {
        printf("Executing task (Thread ID: %d, Priority: %d): %s\n", task->tid, task->priority, task->command);

        // Executing task's command
        int status = system(task->command);

        if (WIFSTOPPED(status)) {
            printf("Task (Thread ID: %d) stopped by signal %d\n", task->tid, WSTOPSIG(status));
        } else if (WIFEXITED(status)) {
           printf("Task (Thread ID: %d) exited with status %d\n", task->tid, WEXITSTATUS(status)); 
        }else if (WIFSIGNALED(status)) {
            printf("Task (Thread ID: %d) terminated by signal %d\n", task->tid, WTERMSIG(status));
        } 
    }
}

// scheduler's Task management functions 
void new_task_addition(const char *command, int priority) {
    // searching in arrays_of_tasks for available task 
    Task *task = NULL;
    pthread_mutex_lock(&tmutex);
    for (int i = 1; i <= GivenCpu; i++) {
    int j=i-1;
        if (arrays_of_tasks[j].command == NULL) {
            task = &arrays_of_tasks[j];
            break;
        }
    }
    if (task != NULL) {
        task->priority = priority;
        task->command = strdup(command);
    }
    pthread_cond_signal(&ReadyTask);
    pthread_mutex_unlock(&tmutex);
}


void *task_thread(void *arg) {
    Task *task = (Task *)arg;
    while (true) {
        pthread_mutex_lock(&tmutex);
        while (task->command == NULL) {
            pthread_cond_wait(&ReadyTask, &tmutex);
            if(task->command != NULL) { break;}
        }
        pthread_mutex_unlock(&tmutex);
        executing_task(task);
        free(task->command);
        task->command = NULL;
    }
    return NULL;
}

typedef struct {
     time_t end_time;      
    time_t start_time; 
     pid_t pid;
    int status;     // 1 -> terminated  0 -> running  
} Job;

Job background_jobs[64]; // Track background jobs for each CPU

void background_job_tracker() {
    int i;
    for (i = 1; i <= GivenCpu; i++) {
        int j=i-1;
        if (background_jobs[j].status == 0) {
            int status;
            pid_t wpid = waitpid(background_jobs[j].pid, &status, WNOHANG);
            if (wpid > 0) {
                background_jobs[j].end_time = time(NULL);
                background_jobs[j].status = 1;
                printf("Background Job -> (PID %d)  is completed.\n", background_jobs[j].pid);
            }
        }
    }
}
// implementing logic for Scheduling task
int current_index_task = 0;

void scheduler_round_robin() {
    while (1) {
        //  priority-based task scheduling Implementation

        //  tmutex Locked to access  priority queue
        pthread_mutex_lock(&tmutex);

        // priority queue empty ?
        if (check_empty_priority_queue()) {
            // If  queue -> empty then wait for tasks and release the lock
            pthread_mutex_unlock(&tmutex);
            usleep(100000); // wait for short time before checking again
        } else {
            // take highest priority task 
            Task *task = getting_top_priority_task();

            //  signal the task to start running by Releasing the lock
            pthread_mutex_unlock(&tmutex);

            // Signal the task to start running
            pthread_cond_signal(&ReadyTask);

            // Sleep till  (TSLICE)
            usleep(TSLICE * 1000); // conversion of TSLICE  milliseconds->microseconds

            pthread_mutex_lock(&tmutex);

            // Add  task at back of priority queue
            new_task_addition_back_to_priority_queue(task);

            // Releasing the lock
            pthread_mutex_unlock(&tmutex);
        }
    }
}



void initialize_scheduler(int GivenCpu) {
    arrays_of_tasks = malloc(GivenCpu * sizeof(Task));
    if (arrays_of_tasks == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < GivenCpu; i++) {
        arrays_of_tasks[i].tid = i;
        arrays_of_tasks[i].command = NULL;
        pthread_create(&(arrays_of_tasks[i].thread), NULL, task_thread, &(arrays_of_tasks[i]));
    }
}