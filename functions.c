//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

#include "functions.h"
#include "shared_mem.h"
#include <time.h>

//Config file gesture
int *read_config_file(){

    /* Config int[] format
    0 - queue_slot_number (must be >= 1);
    1 - num_workers (must be >= 1);
    2 - max_keys (must be >= 1);
    3 - max_sensors (must be >= 1);
    4 - max_alerts (must be >= 0);
    */

    FILE *conf_file = fopen("config.txt", "r");

    if(conf_file == NULL)
        return NULL;
    
    
    int *configs = malloc(sizeof(int) * number_of_configs);
    int current_index = 0;

    char *line = malloc(sizeof(char) * line_lenght);

    while(fgets(line,line_lenght, conf_file) != NULL){
        int value = strtol(line, NULL, 10);

        switch (current_index) {
            case 0: // queue_slot_number (must be >= 1)
                if (value < 1) {
                    fclose(conf_file);
                    free(line);
                    free(configs);
                    return NULL;
                }
                break;

            case 1: // num_workers (must be >= 1)
                if (value < 1) {
                    fclose(conf_file);
                    free(line);
                    free(configs);
                    return NULL;
                }
                break;

            case 2: // max_keys (must be >= 1)
                if (value < 1) {
                    fclose(conf_file);
                    free(line);
                    free(configs);
                    return NULL;
                }
                break;

            case 3: // max_sensors (must be >= 1)
                if (value < 1) {
                    fclose(conf_file);
                    free(line);
                    free(configs);
                    return NULL;
                }
                break;

            case 4: // max_alerts (must be >= 0)
                if (value < 0) {
                    fclose(conf_file);
                    free(line);
                    free(configs);
                    return NULL;
                }
                break;

            default: // too many lines in file
                fclose(conf_file);
                free(line);
                free(configs);
                return NULL;
        }

        configs[current_index++] = value;
    }

    if (current_index != number_of_configs) { // too few lines in file
        fclose(conf_file);
        free(line);
        free(configs);
        return NULL;
    }

    free(line);
    fclose(conf_file);
    return configs;
}


void process_config_file(int *configs){
  config = malloc(sizeof(config_struct));
  config->queue_slot_number = configs[0];
  config->num_workers = configs[1];
  config->max_keys = configs[2];
  config->max_sensors= configs[3];
  config->max_alerts = configs[4];
}

//Debug
void print_config_file(){
    printf("%d\n", config->queue_slot_number);
    printf("%d\n", config->num_workers);
    printf("%d\n", config->max_keys);
    printf("%d\n", config->max_sensors);
    printf("%d\n", config->max_alerts);
}

void print(char *result){
    char time_str[20];

    time_t timer = time(NULL);
    struct tm* tm_info = localtime(&timer);

    strftime(time_str, 20, "%H:%M:%S", tm_info);

    sem_wait(log_semaphore);
    fprintf(log_file, "%s:%s\n",time_str,result);
    printf("%s:%s\n",time_str,result);
    fflush(log_file);
    fflush(stdout);
    sem_post(log_semaphore);
}

char *read_from_pipe(int pipe_fd){

    char buffer[1024];
    ssize_t bytes_read = read(pipe_fd, buffer, sizeof(buffer));

    if (bytes_read == -1) {
        return NULL;
    } else {
        char* data = malloc(bytes_read + 1);
        memcpy(data, buffer, bytes_read);
        data[bytes_read] = '\0';
        return data;
    }
}

// Function to create a new node
struct Node* createNode(char *command) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->command = command;
    newNode->next = NULL;
    return newNode;
}

// Function to create a new queue
struct Queue* createQueue() {
    struct Queue* newQueue = (struct Queue*)malloc(sizeof(struct Queue));
    newQueue->front = newQueue->rear = NULL;
    return newQueue;
}

// Function to check if the queue is empty
int isEmpty(struct Queue* queue) {
    return (queue->front == NULL);
}

// Function to add an element to the rear of the queue
void enqueue(struct Queue* queue, char *command) {
    char *command_copy = strdup(command); 
    struct Node* newNode = createNode(command_copy);
    if (isEmpty(queue)) {
        queue->front = queue->rear = newNode;
        return;
    }
    queue->rear->next = newNode;
    queue->rear = newNode;
}

char* dequeue(struct Queue* queue) {
    if (isEmpty(queue)) {
        return NULL;
    }
    struct Node* temp = queue->front;
    char* command = (char*) malloc(strlen(temp->command) + 1);
    strcpy(command, temp->command);
    queue->front = temp->next;
    free(temp->command);
    free(temp);
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    return command;
}


int queue_size(struct Queue* queue) {
    int count = 0;
    struct Node* current = queue->front;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

void printQueue(struct Queue* queue) {
    if (isEmpty(queue)) {
        printf("Queue is empty.\n");
        return;
    }
    struct Node* current = queue->front;
    printf("Queue contents:\n");
    while (current != NULL) {
        printf("%s\n", current->command);
        current = current->next;
    }
}

void destroyQueue(struct Queue* queue) {
    struct Node* current = queue->front;
    while (current != NULL) {
        struct Node* next = current->next;
        free(current->command);
        free(current);
        current = next;
    }
    free(queue);
}
