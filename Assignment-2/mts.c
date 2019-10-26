#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define BILLION 1000000000.0

pthread_mutex_t station, track;
pthread_cond_t start_loading, start_crossing_n;

/*
*   Struct to hold train information
*/
typedef struct Train {
    int number;
    char direction;
    int priority;
    int loading_time;
    int crossing_time;
} train;

/*
*   Counts the number of lines in the trains.txt file
*   Returns the number of trains
*/
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

/*
*   Returns the priority of a train
*   1 = High Priority
*   0 = Low Priority
*/
int priority(char direction) {
    if (direction == 'W' || direction == 'E')
        return 1;
    else
        return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Error: Add a trains.txt file.\n");
        return 1;
    }
    
    // Get the # of trains
    int n = countLines(argv[1]);
    train *trains;
    trains = malloc(n * sizeof(*trains));

    char direction[1];
    char loading_time[3], crossing_time[3];
    int train_number = 0;
    FILE *fp = fopen(argv[1], "r");
    while (fscanf(fp, "%s %s %s", direction, loading_time, crossing_time) == 3) {
        trains[train_number].number = train_number;
        trains[train_number].direction = direction[0];
        trains[train_number].priority = priority(direction[0]);
        trains[train_number].loading_time = atoi(loading_time);
        trains[train_number].crossing_time = atoi(crossing_time);
        train_number = train_number + 1;
    }
}