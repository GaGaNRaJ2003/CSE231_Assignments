#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>


#define RESET  "\033[0m"
#define BOLD_YLW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLD_BLU    "\033[1m\033[34m"      /* Bold Blue */
#define BOLD_MAG "\033[1m\033[35m"      /* Bold Magenta */
#define BOLD_CYN    "\033[1m\033[36m"      /* Bold Cyan */
#define ANSI_COLOR_LIGHT_GREEN  "\x1b[92m"
#define BOLD_ORG    "\033[1m\033[33m"      /* Bold Orange */
#define ANSI_COLOR_LIGHT_RED    "\x1b[91m"
#define BOLD_FUS    "\033[1m\033[31m"      /* Bold Fuchsia */


// Function prototypes
char *lsh_line_reading(void);
char **lsh_line_spliting(char *line);
int lsh_systemCall(char **arguments);
int echo_systemCall(char **arguments);
int ls_systemCall(char **arguments);
int grep_systemCall(char **arguments);
int wc_systemCall(char **arguments);

volatile sig_atomic_t background_process_count = 0;

#define Line_Buffer_Size 1280
#define token_Buffer_Size 80
#define HISTORY_SIZE 150

char *history[HISTORY_SIZE];
int count_history = 0;

char *previous_command = NULL;

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
    int i, status;
    pid_t wpid;
    i=0;
    while (i < count_history) {
        pid_t process_id;
        struct timeval t0_time, end_t;

        gettimeofday(&t0_time, NULL);
        process_id = fork();

        if (process_id == 0) {
            char **arguments = lsh_line_spliting(history[i]);
            lsh_systemCall(arguments);
            free(arguments);
            exit(EXIT_SUCCESS);
        } if (process_id < 0) {
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
        }i++;
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

int main(int argc, char **argv) {
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
}


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