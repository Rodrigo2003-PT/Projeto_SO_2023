//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

#ifndef functions   /* Include guard */
#define functions

//C libraries
#include "shared_mem.h"

//User define variables
#define line_lenght 20
#define number_of_configs 5

#define LOG_SEM_NAME "log_semaphore"

//Config file gesture
int *read_config_file();
void print_config_file();
char *read_from_pipe(int pipe_fd);
void process_config_file(int *configs);

//Queue
struct Queue* createQueue();
int isEmpty(struct Queue* queue);
char* dequeue(struct Queue* queue);
int queue_size(struct Queue* queue);
void printQueue(struct Queue* queue);
void write_Queue(struct Queue* queue);
struct Node* createNode(char *command);
void destroyQueue(struct Queue* queue);
void enqueue(struct Queue* queue, char *command);


//Log_writing || Dispatcher
void print(char *result);
void process_dispatcher_message(char *msg, int pipes[][2]);


#endif 
