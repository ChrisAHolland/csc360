#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define OUTPUT printf("%d\n", i)

int main() {
    int i = 0; OUTPUT;

    if (fork()) {
        wait(NULL);
        i+=3; OUTPUT;
    } else {
        i+=1; OUTPUT;
        return 0;
    }
}