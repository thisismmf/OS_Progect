#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

typedef struct {
    unsigned long long rx_bytes;
    unsigned long long tx_bytes;
    unsigned int rx_sessions;
    unsigned int tx_sessions;
    time_t start_time;
} NetworkStats;

NetworkStats netStats;

void initNetworkStats() {
    netStats.rx_bytes = 0;
    netStats.tx_bytes = 0;
    netStats.rx_sessions = 0;
    netStats.tx_sessions = 0;
    netStats.start_time = time(NULL);
}

void updateNetworkStats() {
    // Simulate network stats update (should be replaced with real data gathering)
    netStats.rx_bytes += rand() % 1000;
    netStats.tx_bytes += rand() % 1000;
    netStats.rx_sessions += rand() % 10;
    netStats.tx_sessions += rand() % 10;
}

void displayNetworkStats() {
    updateNetworkStats();

    printf("Volume of incoming traffic: %llu bytes\n", netStats.rx_bytes);
    printf("Volume of outgoing traffic: %llu bytes\n", netStats.tx_bytes);
    printf("Number of incoming sessions: %u\n", netStats.rx_sessions);
    printf("Number of outgoing sessions: %u\n", netStats.tx_sessions);
    printf("Activity duration: %ld seconds\n", time(NULL) - netStats.start_time);
}

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

void runForegroundCommand(const char *command) {
    // Execute the command in the foreground
    int status = system(command);

    if (status == -1) {
        printf("Failed to execute command\n");
    } else {
        printf("Command executed in the foreground\n");
    }
}

void redirectOutputToFile(const char *filename, const char *command) {
    // Check if the command is valid
    if (system(command) != 0) {
        printf("Invalid command, unable to redirect output to file\n");
        return;
    }

    // Formulate the command to redirect output to the file
    char redirectCommand[1024];
    sprintf(redirectCommand, "%s > %s", command, filename);

    // Execute the command with output redirected to the file
    int status = system(redirectCommand);

    if (status == -1) {
        printf("Failed to execute command\n");
    } else {
        printf("Output redirected to file '%s'\n", filename);
    }
}

char* replaceLocalVariables(const char *input) {
    // Allocate memory for the modified input string
    char *modifiedInput = (char *)malloc(strlen(input) + 1);
    if (modifiedInput == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    // Copy the input string to the modified input string
    strcpy(modifiedInput, input);

    // Search for local variables prefixed with '$' and replace them
    char *token = strtok(modifiedInput, " $");
    while (token != NULL) {
        // Check if the token starts with '$'
        if (token[0] == '$') {
            // Extract the variable name (without '$')
            char* varName = token + 1;

            // Search for the variable by name
            for (int i = 0; i < numVariables; i++) {
                if (strcmp(variables[i].name, varName) == 0) {
                    // Replace the variable with its value
                    strcpy(token, variables[i].value);
                    break;
                }
            }
        }

        // Find the next token
        token = strtok(NULL, " $");
    }

    return modifiedInput;
}

void displaySystemInformation() {
    // Display processor model
    printf("Processor Model:\n");
    system("sysctl -n machdep.cpu.brand_string");

    // Display number of processor cores
    printf("Number of Processor Cores:\n");
    system("sysctl -n hw.ncpu");

    // Display memory information
    printf("Memory Information:\n");
    system("sysctl -n hw.memsize");

    // Display used and free memory size
    printf("Used and Free Memory Size:\n");
    system("vm_stat | grep 'Pages active' | awk '{print $3 * 4096 / (1024 * 1024) \" MB Used\"}'");
    system("vm_stat | grep 'Pages free' | awk '{print $3 * 4096 / (1024 * 1024) \" MB Free\"}'");

    // Display kernel version
    printf("Kernel Version:\n");
    system("uname -r");

    // Display user interface type
    printf("User Interface Type:\n");
    system("echo $XDG_SESSION_TYPE");
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
    } else if (strstr(input, "&") != NULL) {
        // Tokenize input to extract the command
        char* token = strtok(input, " &\n");

        if (token != NULL) {
            char* command = token;

            // Run the command in the foreground
            runForegroundCommand(command);
        } else {
            printf("Invalid syntax\n");
        }
    } else if (strstr(input, "<") != NULL) {
        // Tokenize input to extract the file name and command
        char* token = strtok(input, " <\n");

        if (token != NULL) {
            char* filename = token;

            token = strtok(NULL, "<\n");
            if (token != NULL) {
                char* command = token;

                // Redirect output of the command to the file
                redirectOutputToFile(filename, command);
            } else {
                printf("Invalid syntax\n");
            }
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
    } else if (strstr(input, "وضعیت پردازه تعاملی") != NULL) {
        // Display interactive processes
        system("ps -eo pid,tty,comm | grep -v '?'");
    } else if (strstr(input, "وضعیت پردازه غیر تعاملی") != NULL) {
        // Display non-interactive processes
        system("ps -eo pid,tty,comm | grep '?'");
    } else if (strstr(input, "وضعیت پردازه ریسه") != NULL) {
        // Display programs with multiple threads
        system("ps -eo pid,nlwp,comm | awk '$2 > 1'");
    } else if (strstr(input, "اطلاعات سیستم") != NULL) {
        // Display system information
        displaySystemInformation();
    } else if (strstr(input, "وضعیت پردازه") != NULL) {
        // Display the list of processes in descending order of priority
        system("ps -eo pid,pri,comm | sort -k 2 -nr");
    } else if (strstr(input, "وضعیت شبکه") != NULL) {
        // Display network status
        displayNetworkStats();
    } else {
        printf("Invalid command\n");
    }
}

int main() {
    // Initialize network statistics
    initNetworkStats();

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
