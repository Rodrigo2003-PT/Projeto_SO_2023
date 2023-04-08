#ifndef functions   /* Include guard */
#define functions

//C libraries
#include "shared_mem.h"

//User define variables
#define line_lenght 20
#define number_of_configs 5

#define LOG_SEM_NAME "log_semaphore"

//Config file gesture
char *read_from_pipe();
int *read_config_file();
void print_config_file();
void process_config_file(int *configs);

//Queue
struct Queue* createQueue();
int isEmpty(struct Queue* queue);
char* dequeue(struct Queue* queue);
int queue_size(struct Queue* queue);
struct Node* createNode(char *command);
void enqueue(struct Queue* queue, char *command);


//Log_writing
void print(char *result);


#endif 
