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
*   Print the prompt as it changes
*/
void printPrompt() {
    char prompt[256];
    char *login;
    login = getlogin();
    char hostname[128];
    gethostname(hostname, sizeof(hostname));
    char cwd[128];
    getcwd(cwd, sizeof(cwd));
    printf("SSI: %s@%s: %s > ", login, hostname, cwd);
}    

void run_command(char** commands) {
    pid_t pid = fork();

    // Inside child proces
    if (pid == 0) {
        execvp(commands[0], commands);
    } else {
        waitpid(pid, NULL, 0); // Change to 1 to wait in background
    }
}
/*
char* processInput(char input[], char* container[]) {
    char *token;
    int i = 0;
    token = strtok(input, " ");
    container[i++] = token;
    while(token != NULL) {
        container[i++] = token;
        token = strtok(NULL, " ");
    }
    return token;
}
*/
int main() {
    for(;;) {
        // Print the prompt and retrieve user input
        //char input[MAX_INPUT];
        printPrompt();
        //fgets(input, MAX_INPUT, stdin);
	char prompt[] = "";
        char* input = readline(prompt);
        // Tokenize the user input
        char* args = strtok(input, " ");
        char* tokens[256];

        int i = 0;
        while (args) {
            tokens[i++] = args;
            args = strtok(NULL, " ");
        }
        tokens[i] = NULL;

        run_command(tokens);
    }

    return 0;
}
