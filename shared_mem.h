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
#define MAX_KEY_SIZE 32
#define MIN_KEY_SIZE 3

#define MAX_ID_LENGTH 32
#define MAX_COMMAND_LENGTH 50
#define MAX_RESPONSE_LENGTH 200

#define PIPENAME "SENSOR_PIPE"

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
    int alert_min;
    int alert_max;
    int alert_flag;
}sensor_alerts;

typedef struct sensor_struct {
    char* id;
    sensor_alerts alerts[3];
    sensor_chave data[6];
    int intervalo;
}sensor_struct;

typedef struct worker_message{
    sensor_struct sens;
} worker_message;

//Struct to save config files
config_struct *config;

//Main PID
pid_t main_pid;

//Shared memory id
int shm_id;

//Message Queue ID
int msq_id;

//Log file management
FILE *log_file;
sem_t *log_semaphore;

//Shared memory locations
sensor_struct *sensor;
int *first_worker;
int count_key;

//Processes PIDS
pid_t alerts_watcher_process;
pid_t worker_process;

#endif
