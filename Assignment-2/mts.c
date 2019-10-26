#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define BILLION 1000000000.0

typedef struct Train {
    int number;
    char direction;
    int priority;
    int loading_time;
    int crossing_time;
} train;

int countLines(char *f) {
    FILE *fp = fopen(f, "r");
    char c;
    int count = 0;
    for (c = fgetc(fp); c != EOF; c = fgetc(fp)) {
        if (c == '\n') count = count + 1;
    }
    fclose(fp);
    return count;
}

int main(int argc, char *argv[]) {
    int n;
    n = countLines(argv[1]);
    printf("%d\n", n);
    return 0;
}