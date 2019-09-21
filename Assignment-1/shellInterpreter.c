#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>

#define MAX_INPUT 128

/*
*   Print the prompt as it changes
*/
void printPrompt() {
    char *login;
    login = getlogin();
    char hostname[128];
    gethostname(hostname, sizeof(hostname));
    char cwd[128];
    getcwd(cwd, sizeof(cwd));
    printf("SSI: %s@%s: %s > ", login, hostname, cwd);
}

int main() {
    char input[MAX_INPUT];

    for(;;) {
        printPrompt();
        fgets(input, MAX_INPUT, stdin);
    }

    return 0;
}
