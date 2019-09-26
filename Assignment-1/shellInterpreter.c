#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_INPUT 128

/*
*   Creates and returns the prompt
*/
char* printPrompt() {
    static char prompt[256];
    char *login;
    char hostname[128];
    char cwd[128];
    login = getlogin();
    gethostname(hostname, sizeof(hostname));
    getcwd(cwd, sizeof(cwd));

    prompt[0] = '\0';
    strcat(prompt, "SSI: ");
    strcat(prompt, login);
    strcat(prompt, "@");
    strcat(prompt, hostname);
    strcat(prompt, cwd);
    strcat(prompt, " > ");

    return(prompt);
}    

// Runs a simple command (Part 1)
void runCommand(char** commands) {
    pid_t pid = fork();

    // Inside child proces
    if (pid == 0) {
        execvp(commands[0], commands);
    } else {
        waitpid(pid, NULL, 0); // Change to 1 to wait in background
    }
}

void changeDirectory(char** path) {
    int ret;

    // Cases to navigate home
    if (path[1] == NULL || !strcmp(path[1], "~")) {
        ret = chdir(getenv("HOME"));
    } else {
        ret = chdir(path[1]);
    }
}

int main() {
    for(;;) {
        // Print the prompt and retrieve user input
        char* input = readline(printPrompt());
        // Tokenize the user input
        char* args = strtok(input, " ");
        char* tokens[256];

        int i = 0;
        while (args) {
            tokens[i++] = args;
            args = strtok(NULL, " ");
        }
        tokens[i] = NULL;

        // Check if its a "cd" command
        if (!strcmp(tokens[0], "cd")) {
            changeDirectory(tokens);
        } else {
            runCommand(tokens);
        }

        free(input);
    }
    return 0;
}
