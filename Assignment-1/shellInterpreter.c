/*
*   A simple shell interpreter.
*   ~ Chris Holland
*/
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
*   Linked list node
*/
typedef struct Node {
    char command[1024];
    pid_t pid;
    struct Node* next;
} node;

// Create head node for the list
node* head = NULL;

// Number of nodes in the list
int listLength = 0;

/*
*   Adds a new node to the start of the list
*/
void add(char** command, pid_t pid) {
    node* newNode = (node*)malloc(sizeof(node));
    newNode->pid = pid;
    newNode->command[0] = '\0';
    int i = 1;

    while(command[i] != NULL) {
        strcat(newNode->command, command[i]);
        strcat(newNode->command, " ");
        i++;
    }
    newNode->next = head;
    head = newNode;
    listLength++;
}

/*
*   Deletes the node from the list with a given pid
*   Based on the pseudocode provided in Tutorial 3 (Slide 5)
*/
void delete(pid_t pid) {
    node* temp = head;
    
    if (head->pid == pid) {
        printf("%d: %s has terminated.\n", temp->pid, temp->command);
        head = head->next;
    } else {
        while(temp->next->pid != pid) 
            temp = temp->next;

        printf("%d: %s has terminated.\n", temp->next->pid, temp->next->command);
        temp->next = temp->next->next;
        free(temp->next);
    }
    listLength--;
}

/*
*   Lists the currently running background commands
*/
void listNodes() {
    node* temp = head;

    while(temp != NULL) {
        printf("%d: %s\n", temp->pid, temp->command);
        temp = temp->next;
    }

    printf("Total Background Jobs: %d\n", listLength);
}

/*
*   Check if a child process has terminated
*   Based on the pseudocode provided in Tutorial 3 (Slide 5)
*/
void checkProcesses() {
    if (listLength > 0) {
        pid_t pid = waitpid(0, NULL, WNOHANG);

        while(pid > 0) {
            // Unlike aformentioned pseudocode, I implemented a seperate delete function
            delete(pid);
            pid = waitpid(0, NULL, WNOHANG);
        }
    }
}

/*
*   Creates the prompt in the buffer
*/
void getPrompt(char* prompt) {
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

// Handles the cd command
void changeDirectory(char** path) {
    int ret;

    // Cases to navigate home
    if (path[1] == NULL || !strcmp(path[1], "~")) {
        ret = chdir(getenv("HOME"));
    } else if (!strcmp(path[1], "..")) {
        ret = chdir("../");
    } else {
        ret = chdir(path[1]);
    }
}

void addBackground(char** commands) {
    pid_t pid = fork();

    if (pid == 0) {
        execvp(commands[1], commands + 1);
    } else if (pid > 0) {
        add(commands, pid);
    }
}

int main() {
    for(;;) {
        // Print the prompt and retrieve user input
        char prompt[256];
        getPrompt(prompt);
        char* input = readline(prompt);

        // Had a bug where if I just hit 'Enter' I would get a seg. fault.
        // This fixes that
        if (*input == '\0') {
            free(input);
            continue;
        }
        // Tokenize the user input
        char* args = strtok(input, " ");
        char* tokens[MAX_INPUT];

        // Check for terminating processes
        checkProcesses();

        // Tokenize input into command tokens
        int i = 0;
        while (args) {
            tokens[i++] = args;
            args = strtok(NULL, " ");
        }
        tokens[i] = NULL;

        // Check if it's a "cd" command (Part 2)
        if (!strcmp(tokens[0], "cd")) {
            changeDirectory(tokens);
        // Check if it's a command to run in the background (Part 3)
        } else if (!strcmp(tokens[0], "bg")) {
            addBackground(tokens);
        // List the background processes (Part 3)
        } else if (!strcmp(tokens[0], "bglist")) {
            listNodes();
        // Command to exit the SSI
        } else if (!strcmp(tokens[0], "exit")) {
            printf("You have chosen to exit the Simple Shell Interpreter.\n");
            exit(1);
        } else {
        // Default to run a basic command (Part 1)
            runCommand(tokens);
        }

        free(input);
    }
    return 0;
}
