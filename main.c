#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

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

// Function to replace the last n characters of a string with '_'
void maskSensitiveInfo(char *str, int n) {
    int len = strlen(str);
    if (n > len) return; // If n is greater than length of string, do nothing

    for (int i = len - n; i < len; i++) {
        str[i] = '_';
    }
}

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

void listDirectory(const char *path) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(path))) {
        perror("opendir error");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Mask sensitive info for files and directories
        maskSensitiveInfo(entry->d_name, 6);
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
}

void parseAndExecuteCommand(char *input) {
    // Check if the command is to define a local variable
    if (strstr(input, "تنظیمات") != NULL) {
        // Tokenize input to extract the variable name
        char *token = strtok(input, " تنظیمات\n");

        if (token != NULL) {
            char *name = token;

            // Search for the variable by name
            int found = 0;
            for (int i = 0; i < numVariables; i++) {
                if (strcmp(variables[i].name, name) == 0) {
                    printf("%s\n", variables[i].value);
                    found = 1;
                    break;
                }
            }

            if (!found) {
                printf("Variable '%s' not found\n", name);
            }
        } else {
            printf("Invalid syntax\n");
        }
    } else if (strstr(input, "تنظیم") != NULL) {
        // Tokenize input to separate the variable name and value
        char *token = strtok(input, " =تنظیم\n");

        if (token != NULL) {
            char *name = token;

            // Find the index of the equal sign
            token = strtok(NULL, " =تنظیم\n");
            if (token != NULL) {
                char *value = token;

                // Store the variable
                if (numVariables < MAX_VARIABLES) {
                    strncpy(variables[numVariables].name, name, sizeof(variables[numVariables].name) - 1);
                    strncpy(variables[numVariables].value, value, sizeof(variables[numVariables].value) - 1);
                    variables[numVariables].name[sizeof(variables[numVariables].name) - 1] = '\0';
                    variables[numVariables].value[sizeof(variables[numVariables].value) - 1] = '\0';
                    numVariables++;
                    printf("Variable %s set to %s\n", name, value);
                } else {
                    printf("Maximum number of variables reached\n");
                }
            } else {
                printf("Invalid syntax\n");
            }
        } else {
            printf("Invalid syntax\n");
        }
    } else if (strstr(input, "فهرست_مخفی") != NULL) {
        // Tokenize input to extract the directory path
        char *token = strtok(input, " فهرست_مخفی\n");

        if (token != NULL) {
            char *path = token;

            // List directory contents with masked sensitive info
            listDirectory(path);
        } else {
            printf("Invalid syntax\n");
        }
    } else if (strstr(input, "فهرست") != NULL) {
        // Tokenize input to extract the directory path
        char *token = strtok(input, " فهرست\n");

        if (token != NULL) {
            char *path = token;

            // Open the directory
            DIR *dir = opendir(path);
            if (dir == NULL) {
                printf("Error opening directory '%s'\n", path);
                return;
            }

            // Read directory contents
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                printf("%s\n", entry->d_name);
            }

            // Close the directory
            closedir(dir);
        } else {
            printf("Invalid syntax\n");
        }
    } else if (strstr(input, "برو") != NULL) {
        // Tokenize input to extract the directory path
        char *token = strtok(input, " برو\n");

        if (token != NULL) {
            char *path = token;

            // Change directory
            if (chdir(path) != 0) {
                printf("Error changing directory to '%s'\n", path);
            }
        } else {
            printf("Invalid syntax\n");
        }
    } else if (strstr(input, "محتوا") != NULL) {
        // Tokenize input to extract the file path
        char *token = strtok(input, " محتوا\n");

        if (token != NULL) {
            char *path = token;

            // Open the file
            FILE *file = fopen(path, "r");
            if (file == NULL) {
                printf("Error opening file '%s'\n", path);
                return;
            }

            // Read and print file contents
            char line[1024];
            while (fgets(line, sizeof(line), file) != NULL) {
                printf("%s", line);
            }

            // Close the file
            fclose(file);
        } else {
            printf("Invalid syntax\n");
        }
    } else {
        printf("Invalid command\n");
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

        parseAndExecuteCommand(input);
    }

    return 0;
}