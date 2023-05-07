//Rodrigo Sá 2021213188
//Miguel Miranda 2021212100

#ifndef shared_mem   /* Include guard */
#define shared_mem

//imports 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>

#define MAX_SENSOR_ID_SIZE 32
#define MIN_SENSOR_ID_SIZE 3
#define ALERTS_PER_SENSOR 4
#define MESSAGE_SIZE 1024
#define MAX_KEY_SIZE 32
#define MIN_KEY_SIZE 3


#define PIPENAME_1 "SENSOR_PIPE"

#define PIPENAME_2 "CONSOLE_PIPE"

#define MSQ_FILE "msq_id.txt"

typedef struct config_struct{
    int queue_slot_number;
    int num_workers;
    int max_keys;
    int max_sensors;
    int max_alerts;
}config_struct;

typedef struct sensor_chave{
    char* chave;
    int last_value;
    int min_value;
    int max_value;
    int count;
    double avg;
}sensor_chave;

typedef struct sensor_alerts{
    pid_t pid;
    int alert_min;
    int alert_max;
    int alert_flag;
    char* alert_id;
}sensor_alerts;

typedef struct sensor_struct {
    char* id;
    sensor_alerts alerts[ALERTS_PER_SENSOR];
    sensor_chave data;
}sensor_struct;

typedef struct alert_msg {
    long mtype;
    char sensor_id[MAX_SENSOR_ID_SIZE];
    int triggered_value;
} alert_msg;

typedef struct Node {
    char *command;
    struct Node* next;
}Node;

typedef struct Queue {
    struct Node* front;
    struct Node* rear;
}Queue;

// Queue Pointer
struct Queue* queue;

//Struct to save config files
config_struct *config;

//Message Queue ID
int msq_id;

//Shared memory id
int shm_id;
sem_t *array_sem;
sem_t *alerts_sem;
sem_t *worker_sem; 

//Log file management
FILE *log_file;
sem_t *log_semaphore;

//Shared memory locations
sensor_struct *sensor;
int *first_worker;
int count_key;

//Processes PIDS
pid_t alerts_watcher_process;
pid_t* worker_pid;

//Threads
pthread_t console_reader_thread, sensor_reader_thread, dispatcher_thread, console_thread, console_receive;

int running;

#endif
