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
    char command[256];
    pid_t pid;
    struct Node* next;
} node;

// Create head node for the list
node* head = NULL;

// Number of nodes in the list
int listLength = 0;

/*
*   Adds a new node to the list
*/
void add(char** command, pid_t pid) {
    node* newNode = (node*)malloc(sizeof(node));
    newNode->pid = pid;
    int i = 0;

    while(command[i] != NULL) {
        strcat(newNode->command, command[i]);
        strcat(newNode->command, " ");
        i++;
    }

    if (listLength == 0) {
        head = newNode;
    } else {
        node* temp = head;

        while(temp->next != NULL) 
            temp = temp->next;
        
        temp->next = newNode;
    }
    newNode->next = NULL;
    listLength++;
}

void delete(pid_t pid) {
    node* temp = head;
    listLength--;
    
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
}

/*
*   Lists the currently running background commands
*/
void listNodes() {
    node* temp = head;

    while(temp!= NULL) {
        printf("%d: %s\n", temp->pid, temp->command);
        temp = temp->next;
    }

    printf("Total Background Jobs: %d\n", listLength);
}

void checkProcesses() {
    if (listLength > 0) {
        pid_t pid = waitpid(0, NULL, WNOHANG);

        while(pid > 0) {
            delete(pid);
            pid = waitpid(0, NULL, WNOHANG);
        }
    }
}

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

// Handles the cd command
void changeDirectory(char** path) {
    int ret;

    // Cases to navigate home
    if (path[1] == NULL || !strcmp(path[1], "~")) {
        ret = chdir(getenv("HOME"));
    } else {
        ret = chdir(path[1]);
    }
}

void addBackground(char** commands) {
    pid_t pid = fork();

    if (pid == 0) {
        execvp(commands[1], commands + 1);
        exit(1);
    } else if (pid > 0) {
        add(commands, pid);
    }
}

int main() {
    for(;;) {
        // Print the prompt and retrieve user input
        char* input = readline(printPrompt());
        // Tokenize the user input
        char* args = strtok(input, " ");
        char* tokens[256];

        // Check for terminating processes
        checkProcesses();

        int i = 0;
        while (args) {
            tokens[i++] = args;
            args = strtok(NULL, " ");
        }
        tokens[i] = NULL;

        // Check if its a "cd" command
        if (!strcmp(tokens[0], "cd")) {
            changeDirectory(tokens);
        } else if (!strcmp(tokens[0], "bg")) {
            addBackground(tokens);
        } else if (!strcmp(tokens[0], "bglist")) {
            listNodes();
        } else {
            runCommand(tokens);
        }

        free(input);
    }
    return 0;
}
