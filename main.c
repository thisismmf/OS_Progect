#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_PATH_LENGTH 1024
#define MAX_USERNAME_LENGTH 64
#define MAX_HOSTNAME_LENGTH 64
#define MAX_VARIABLES 10

typedef struct {
    char name[64];
    char value[256];
} Variable;

Variable variables[MAX_VARIABLES];
int numVariables = 0;

void printPrompt() {
    char path[MAX_PATH_LENGTH];
    char username[MAX_USERNAME_LENGTH];
    char hostname[MAX_HOSTNAME_LENGTH];

    // Get current working directory
    if (getcwd(path, sizeof(path)) == NULL) {
        perror("getcwd() error");
        exit(EXIT_FAILURE);
    }

    // Get logged-in username
    if (getlogin_r(username, sizeof(username)) != 0) {
        perror("getlogin_r() error");
        exit(EXIT_FAILURE);
    }

    // Get hostname
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        perror("gethostname() error");
        exit(EXIT_FAILURE);
    }

    // Print prompt
    printf("%s@%s-%s$ ", username, hostname, path);
}

void parseAndExecuteCommand(char* input) {
    // Tokenize input to separate the variable name and value
    char* token = strtok(input, " =تنظیم\n");

    if (token != NULL) {
        char* name = token;

        // Find the index of the equal sign
        token = strtok(NULL, " =تنظیم\n");
        if (token != NULL) {
            char* value = token;

            // Store the variable
            if (numVariables < MAX_VARIABLES) {
                strncpy(variables[numVariables].name, name, sizeof(variables[numVariables].name) - 1);
                strncpy(variables[numVariables].value, value, sizeof(variables[numVariables].value) - 1);
                variables[numVariables].name[sizeof(variables[numVariables].name) - 1] = '\0';
                variables[numVariables].value[sizeof(variables[numVariables].value) - 1] = '\0';
                numVariables++;
                printf("متغیر %s به مقدار %s تنظیم شد\n", name, value);
            } else {
                printf("به بیشینه مقدار متغیر ها رسیدیم\n");
            }
        } else {
            printf("دستور نامعتبر\n");
        }
    } else {
        printf("دستور نامعتبر\n");
    }
}

int main() {
    while (1) {
        printPrompt(); // Print prompt before each command

        // Read user input
        char input[1024];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; // Exit if user enters EOF (Ctrl+D)
        }

        // Check if the command is to define a local variable
        if (strstr(input, "تنظیم") != NULL) {
            parseAndExecuteCommand(input);
        } else {
            // Process other commands
            printf("شما وارد کردید : %s", input);
        }
    }

    return 0;
}
