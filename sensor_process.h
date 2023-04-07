#ifndef sensor_process   /* Include guard */
#define sensor_process

#include "shared_mem.h"
#include "functions.h"

#define MESSAGE_SIZE 1024
#define MAX_LENGTH 32
#define MIN_LENGTH 3

pid_t sensor_pid;

void handle_sigint(int signal);
void handle_sigtstp(int signal);
void send_message(char* id, char* key, int value);

#endif