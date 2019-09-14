#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>

int main() {
    char *login;
    login = getlogin();
    char hostname[128];
    gethostname(hostname, sizeof(hostname));
    char cwd[128];
    getcwd(cwd, sizeof(cwd));
    printf("SSI: %s@%s: %s >\n", login, hostname, cwd);
    return 0;
}
