#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

// Timer setup from Tutorial 7
#define BILLION 1000000000.0;

struct timespec start, stop;
double accum;

/*
*   Struct to hold train information
*/
typedef struct Train {
    int number;
    char direction;
    int priority;
    int loading_time;
    int crossing_time;
    pthread_cond_t *train_convar;
} train;

/*===========================================
            Priority Queue Code
===========================================*/

/*
*   Queue node struct
*/
typedef struct node {
    train* data;
    int priority;
    struct node* next;
} node;

/*
*   Creates a new node
*/
node* newNode(train* data) {
    node* temp  = (node*)malloc(sizeof(node));
    temp->data = data;
    temp->priority = data->priority;
    temp->next = NULL;
}

/*
*   Returns the node with the highest priority
*/
train* peek(node** head) {
    return (*head)->data;
}

/*
*   Removes the node with the highest priority
*/
void dequeue(node** head) {
    node* temp = *head;
    (*head) = (*head)->next;
    free(temp);
}

/*
*   Determines if the queue is empty
*/
int isEmpty(node** head) {
    return (*head) == NULL;
}

/*
*   Returns the priority of the highest priority node in the queue
*/
int peekPriority(node** head) {
    return (*head)->priority;
}

/*
*   Adds a new train/node to the queue
*/
void enqueue(node** head, train* data) {
    node* start = *head;
    node* temp = newNode(data);

    if (*head == NULL) {
        *head = temp;
        return;
    }

    if ((*head)->priority < temp->priority || 
        ((*head)->data->loading_time == temp->data->loading_time && (*head)->data->number > temp->data->number)) {
        temp->next = *head;
        (*head) = temp;
    } else {
        while (start->next != NULL  && start->next->priority >= temp->priority) {
            if (start->next->data->loading_time == temp->data->loading_time &&
                start->next->data->number > temp->data->number) {
                    temp->next = start->next;
                    start->next = temp;
                    return;
                }
            start = start->next;
        }
        temp->next = start->next;
        start->next = temp;
    }
}

/*===========================================
            MTS Code
===========================================*/

pthread_mutex_t station, track;
pthread_cond_t loaded, start_loading, start_crossing_n;

node* stationWestHead;
node* stationEastHead;

int n;

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

/*
*   Builds the trains in the array of structs
*   from the given input file and array of conditions
*/
void buildTrains(train *trains, char *f, pthread_cond_t *conditions) {
    char direction[3], loading_time[3], crossing_time[3];
    int train_number = 0;
    FILE *fp = fopen(f, "r");
    while (fscanf(fp, "%s %s %s", direction, loading_time, crossing_time) == 3) {
        trains[train_number].number = train_number;
        trains[train_number].direction = direction[0];
        trains[train_number].priority = priority(direction[0]);
        trains[train_number].loading_time = atoi(loading_time);
        trains[train_number].crossing_time = atoi(crossing_time);
        trains[train_number].train_convar = &conditions[train_number];
        train_number = train_number + 1;
    }
    fclose(fp);
}

/*
*   Returns the full value of the direction the train is heading
*/
char* directionString(char dir){
    char* dirString;
    if (dir == 'e' || dir == 'E')
        dirString = "East";
    else
        dirString = "West";
    return dirString;
}

/*
*   Prints the time
*   Based on tutorial slides
*/
void printTime() {
    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) {
        perror("clock gettime");
        exit(EXIT_FAILURE);
    }

    accum = (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec) / BILLION;
    double x = accum*10;
    x = (int)x%60;
    x = (float)x;
    int hours = (int)accum/3600;
    int minutes = (int)accum/60;
    printf("%02d:%02d:%04.1f ", hours, minutes, x/10);
}

/*
*   Main routine for the train threads
*/
void *start_routine(void *args) {
    train* ptrain = (train*)args;
    unsigned int loadTime = (ptrain->loading_time) * 100000;
    usleep(loadTime);
    printTime();
    printf("Train %2d is ready to go %4s.\n", ptrain->number, directionString(ptrain->direction));
    pthread_mutex_lock(&station);
    
    if (ptrain->direction == 'E' || ptrain->direction == 'e') {
        enqueue(&stationEastHead, ptrain);
    } else {
        enqueue(&stationWestHead, ptrain);
    }
    
    pthread_mutex_unlock(&station);
    pthread_cond_signal(&loaded);
    pthread_cond_wait(ptrain->train_convar, &track);
    printTime();
    printf("Train %2d is ON on the main track going %4s\n", ptrain->number, directionString(ptrain->direction));
    unsigned int crossTime = (ptrain->crossing_time) * 100000;
    usleep(crossTime);
    printTime();
    printf("Train %2d is OFF the main track going %4s\n", ptrain->number, directionString(ptrain->direction));
    n--;
    pthread_mutex_unlock(&track);
    pthread_cond_signal(&start_crossing_n);
}

/*
*   Determines which queue to take the new train from
*   0 = East
*   1 = West
*/
int nextTrain(int previous) {
    if (!isEmpty(&stationEastHead) && !isEmpty(&stationWestHead)) {
        if (peekPriority(&stationEastHead) > peekPriority(&stationWestHead)) {
            return 0;
        } else if (peekPriority(&stationEastHead) < peekPriority(&stationWestHead)) {
            return 1;
        }

        if (previous == 1 || previous == -1) {
            return 0;
        } else {
            return 1;
        }
    } else if (!isEmpty(&stationEastHead) && isEmpty(&stationWestHead)) {
        return 0;
    } else if (isEmpty(&stationEastHead) && !isEmpty(&stationWestHead)) {
        return 1;
    }
    return -2;
}

/*
*   Main
*/
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Error: Add a trains.txt file.\n");
        return 1;
    }
    
    // Get the # of trains
    n = countLines(argv[1]);

    pthread_mutex_init(&station, NULL);
    pthread_mutex_init(&track, NULL);
    pthread_t train_threads[n];
    pthread_cond_t train_conditions[n];
    pthread_cond_init(&start_loading, NULL);
    pthread_cond_init(&start_crossing_n, NULL);

    // Build the trains
    train *trains = malloc(n * sizeof(*trains));

    // Initiate the condition variables
    for (int i = 0; i < n; i++) {
        pthread_cond_init(&train_conditions[i], NULL);
    }

    buildTrains(trains, argv[1], train_conditions);

    // Create the threads for each train
    for (int i = 0; i < n; i++) {
        int rc = pthread_create(&train_threads[i], NULL, start_routine, (void *) &trains[i]);
    }

    //startbool = 1;
    if(clock_gettime(CLOCK_REALTIME, &start) == -1) {
        perror("clock gettime");
        exit(EXIT_FAILURE);
    }
    
    int previous = -1;
    while (n != 0) {
        pthread_mutex_lock(&track);

        if (isEmpty(&stationEastHead) && isEmpty(&stationWestHead)) {
            pthread_cond_wait(&loaded, &track);
        }
        previous = nextTrain(previous);

        if (previous == 0) {
            pthread_cond_signal(peek(&stationEastHead)->train_convar);
            pthread_mutex_lock(&station);
            dequeue(&stationEastHead);
            pthread_mutex_unlock(&station);
        } else if (previous == 1) {
            pthread_cond_signal(peek(&stationWestHead)->train_convar);
            pthread_mutex_lock(&station);
            dequeue(&stationWestHead);
            pthread_mutex_unlock(&station);          
        }

        pthread_cond_wait(&start_crossing_n, &track);
        pthread_mutex_unlock(&track);
    }

    free(trains);
    return 0;
}